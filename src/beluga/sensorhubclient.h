// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SENSORHUBCLIENT_H
#define SENSORHUBCLIENT_H

#include <QByteArray>
#include <QObject>
#include <QTimer>

struct gbinder_servicemanager;
struct gbinder_remote_object;
struct gbinder_client;
struct gbinder_local_object;

namespace Coprocessor {

class SensorHubClient : public QObject
{
    Q_OBJECT

public:
    explicit SensorHubClient(QObject *parent = nullptr);
    ~SensorHubClient() override;

    bool connectToService();
    bool isConnected() const;

    bool send(int msgId, const QByteArray &payload = QByteArray());

signals:
    void connected();
    void disconnected();

private:
    bool attach(gbinder_remote_object *object);
    void detach();
    void scheduleRetry();
    void registerMcuService();

    static void onServiceRegistered(gbinder_servicemanager *, const char *name, void *self);
    static void onServiceDied(gbinder_remote_object *, void *self);

    gbinder_servicemanager *m_manager = nullptr;
    gbinder_remote_object *m_object = nullptr;
    gbinder_client *m_client = nullptr;
    gbinder_local_object *m_mcuService = nullptr;
    unsigned long m_registrationHandler = 0;
    unsigned long m_deathHandler = 0;
    QTimer m_retry;
    int m_retryDelayMs = 0;
};

}

#endif
