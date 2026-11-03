// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "sensorhubclient.h"

#include <QDebug>
#include <gbinder.h>

namespace Coprocessor {

namespace {

constexpr const char *BinderDevice = "/dev/binder";
constexpr const char *ServiceName = "deamonserver";
constexpr const char *Interface = "com.oppo.sensorhub.service.IDeamonService";
constexpr guint32 TransactionSendInfo = 1;
constexpr guint32 TransactionRegisterListener = 2;
constexpr guint32 TransactionUnregisterListener = 3;
constexpr const char *ListenerInterface = "com.oppo.sensorhub.service.ISensorDataChangedListener";
constexpr gint32 DomainMcuFramework = 100;
constexpr guint TransactionNotify = 1;
// Without this service every deamonserver reply waits 5 s for it.
constexpr const char *McuServiceName = "mcu";
constexpr const char *McuInterface = "android.app.wear.IMcuManager";
constexpr int MaxPayload = 512;
constexpr int FirstRetryMs = 500;
constexpr int MaxRetryMs = 8000;
constexpr int ListenerRetryMs = 5000;

GBinderLocalReply *onMcuServiceCall(GBinderLocalObject *object, GBinderRemoteRequest *, guint code, guint,
                                    int *status, void *)
{
    qDebug() << "deamonserver called" << McuInterface << "transaction" << code;
    *status = GBINDER_STATUS_OK;
    GBinderLocalReply *reply = gbinder_local_object_new_reply(object);
    gbinder_local_reply_append_int32(reply, 0);
    return reply;
}

}

SensorHubClient::SensorHubClient(QObject *parent)
    : QObject(parent)
{
    m_retry.setSingleShot(true);
    connect(&m_retry, &QTimer::timeout, this, &SensorHubClient::connectToService);
    m_listenerRetry.setSingleShot(true);
    m_listenerRetry.setInterval(ListenerRetryMs);
    connect(&m_listenerRetry, &QTimer::timeout, this, &SensorHubClient::finishConnecting);
}

SensorHubClient::~SensorHubClient()
{
    unregisterListener();
    detach();
    if (m_listener) {
        gbinder_local_object_drop(m_listener);
        m_listener = nullptr;
    }
    if (m_mcuService) {
        gbinder_local_object_drop(m_mcuService);
        m_mcuService = nullptr;
    }
    if (m_manager) {
        if (m_registrationHandler)
            gbinder_servicemanager_remove_handler(m_manager, m_registrationHandler);
        gbinder_servicemanager_unref(m_manager);
    }
}

bool SensorHubClient::isConnected() const
{
    return m_client && m_listenerRegistered;
}

bool SensorHubClient::connectToService()
{
    if (m_client)
        return true;
    if (!m_manager) {
        m_manager = gbinder_servicemanager_new(BinderDevice);
        if (!m_manager) {
            qWarning() << "Unable to open" << BinderDevice;
            return false;
        }
        registerMcuService();
    }
    if (!m_registrationHandler) {
        m_registrationHandler =
            gbinder_servicemanager_add_registration_handler(m_manager, ServiceName, onServiceRegistered, this);
    }
    int status = 0;
    GBinderRemoteObject *object = gbinder_servicemanager_get_service_sync(m_manager, ServiceName, &status);
    if (!object) {
        if (m_retryDelayMs == 0)
            qInfo() << ServiceName << "is not registered (status" << status << "), waiting for it";
        scheduleRetry();
        return false;
    }
    return attach(object);
}

void SensorHubClient::registerMcuService()
{
    // Only acquireWakeLock(int) is called, and only its exception code is read.
    m_mcuService = gbinder_servicemanager_new_local_object(m_manager, McuInterface, onMcuServiceCall, nullptr);
    const int status = gbinder_servicemanager_add_service_sync(m_manager, McuServiceName, m_mcuService);
    if (status != GBINDER_STATUS_OK) {
        qWarning() << "Unable to register" << McuServiceName << "(status" << status
                   << "), messages to the MCU will be slow";
        gbinder_local_object_drop(m_mcuService);
        m_mcuService = nullptr;
    }
}

bool SensorHubClient::registerListener()
{
    if (!m_listener)
        m_listener = gbinder_servicemanager_new_local_object(m_manager, ListenerInterface, onListenerCall, this);

    GBinderLocalRequest *request = gbinder_client_new_request(m_client);
    GBinderWriter writer;
    gbinder_local_request_init_writer(request, &writer);
    gbinder_writer_append_int32(&writer, DomainMcuFramework);
    gbinder_writer_append_local_object(&writer, m_listener);
    int status = 0;
    GBinderRemoteReply *reply =
        gbinder_client_transact_sync_reply(m_client, TransactionRegisterListener, request, &status);
    gbinder_local_request_unref(request);
    if (!reply) {
        qWarning() << "Registering the MCU listener failed, binder status" << status;
        return false;
    }
    GBinderReader reader;
    gbinder_remote_reply_init_reader(reply, &reader);
    gint32 exception = -1;
    gint32 result = 0;
    gbinder_reader_read_int32(&reader, &exception);
    gbinder_reader_read_int32(&reader, &result);
    gbinder_remote_reply_unref(reply);
    if (exception != 0 || result != 1) {
        // Another client already holds the domain.
        qWarning() << "Registering the MCU listener failed, exception" << exception << "result" << result;
        return false;
    }
    m_listenerRegistered = true;
    return true;
}

void SensorHubClient::unregisterListener()
{
    if (!m_client || !m_listenerRegistered)
        return;
    GBinderLocalRequest *request = gbinder_client_new_request(m_client);
    GBinderWriter writer;
    gbinder_local_request_init_writer(request, &writer);
    gbinder_writer_append_int32(&writer, DomainMcuFramework);
    gbinder_writer_append_local_object(&writer, m_listener);
    int status = 0;
    GBinderRemoteReply *reply =
        gbinder_client_transact_sync_reply(m_client, TransactionUnregisterListener, request, &status);
    gbinder_local_request_unref(request);
    if (reply)
        gbinder_remote_reply_unref(reply);
    m_listenerRegistered = false;
}

void SensorHubClient::finishConnecting()
{
    if (!m_client || m_listenerRegistered)
        return;
    if (!registerListener()) {
        m_listenerRetry.start();
        return;
    }
    emit connected();
}

GBinderLocalReply *SensorHubClient::onListenerCall(GBinderLocalObject *, GBinderRemoteRequest *request, guint code,
                                                   guint, int *status, void *self)
{
    *status = GBINDER_STATUS_OK;
    if (code != TransactionNotify)
        return nullptr;

    GBinderReader reader;
    gbinder_remote_request_init_reader(request, &reader);
    gint32 msgId = -1;
    gint32 hasData = 0;
    gint32 length = 0;
    gbinder_reader_read_int32(&reader, &msgId);
    gbinder_reader_read_int32(&reader, &hasData);
    QByteArray payload;
    if (hasData && gbinder_reader_read_int32(&reader, &length) && length > 0) {
        gsize total = 0;
        const auto *data = static_cast<const char *>(gbinder_reader_get_data(&reader, &total));
        // get_data() includes the interface header the reader skipped.
        const gsize position = total - gbinder_reader_bytes_remaining(&reader);
        if (data && length <= MaxPayload && position + length <= total)
            payload = QByteArray(data + position, length);
    }
    auto *client = static_cast<SensorHubClient *>(self);
    // libgbinder calls this on our thread.
    emit client->messageReceived(msgId, payload);
    return nullptr;
}

void SensorHubClient::scheduleRetry()
{
    m_retryDelayMs = m_retryDelayMs ? qMin(m_retryDelayMs * 2, MaxRetryMs) : FirstRetryMs;
    m_retry.start(m_retryDelayMs);
}

bool SensorHubClient::attach(GBinderRemoteObject *object)
{
    m_object = gbinder_remote_object_ref(object);
    m_client = gbinder_client_new(m_object, Interface);
    if (!m_client) {
        qWarning() << "Unable to create a client for" << ServiceName;
        gbinder_remote_object_unref(m_object);
        m_object = nullptr;
        return false;
    }
    m_deathHandler = gbinder_remote_object_add_death_handler(m_object, onServiceDied, this);
    if (m_registrationHandler) {
        gbinder_servicemanager_remove_handler(m_manager, m_registrationHandler);
        m_registrationHandler = 0;
    }
    m_retry.stop();
    m_retryDelayMs = 0;
    qInfo() << "Connected to" << ServiceName;
    finishConnecting();
    return true;
}

void SensorHubClient::detach()
{
    m_listenerRetry.stop();
    m_listenerRegistered = false;
    if (m_object && m_deathHandler) {
        gbinder_remote_object_remove_handler(m_object, m_deathHandler);
        m_deathHandler = 0;
    }
    if (m_client) {
        gbinder_client_unref(m_client);
        m_client = nullptr;
    }
    if (m_object) {
        gbinder_remote_object_unref(m_object);
        m_object = nullptr;
    }
}

void SensorHubClient::onServiceRegistered(GBinderServiceManager *, const char *name, void *self)
{
    auto *client = static_cast<SensorHubClient *>(self);
    qInfo() << name << "registered";
    client->connectToService();
}

void SensorHubClient::onServiceDied(GBinderRemoteObject *, void *self)
{
    auto *client = static_cast<SensorHubClient *>(self);
    qWarning() << ServiceName << "died";
    client->detach();
    emit client->disconnected();
    client->connectToService();
}

bool SensorHubClient::send(int msgId, const QByteArray &payload)
{
    if (!m_client || payload.size() > MaxPayload)
        return false;

    GBinderLocalRequest *request = gbinder_client_new_request(m_client);
    GBinderWriter writer;
    gbinder_local_request_init_writer(request, &writer);
    gbinder_writer_append_int32(&writer, msgId);
    gbinder_writer_append_int32(&writer, 1);
    gbinder_writer_append_int32(&writer, payload.size());
    if (!payload.isEmpty()) {
        // Parcel::read() on the other side skips to the next 4 byte boundary.
        QByteArray padded = payload;
        padded.append((4 - padded.size() % 4) % 4, '\0');
        gbinder_writer_append_bytes(&writer, padded.constData(), padded.size());
    }

    int status = 0;
    GBinderRemoteReply *reply = gbinder_client_transact_sync_reply(m_client, TransactionSendInfo, request, &status);
    gbinder_local_request_unref(request);
    if (!reply) {
        qWarning() << "Sending message" << msgId << "failed, binder status" << status;
        return false;
    }

    GBinderReader reader;
    gbinder_remote_reply_init_reader(reply, &reader);
    gint32 exception = -1;
    gint32 result = 0;
    const bool complete = gbinder_reader_read_int32(&reader, &exception) && gbinder_reader_read_int32(&reader, &result);
    gbinder_remote_reply_unref(reply);
    if (!complete || exception != 0 || result != 1) {
        qWarning() << "Sending message" << msgId << "failed, exception" << exception << "result" << result;
        return false;
    }
    qDebug() << "Sent message" << msgId << "with" << payload.size() << "bytes";
    return true;
}

}
