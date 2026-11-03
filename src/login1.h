// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef LOGIN1_H
#define LOGIN1_H

#include <QDBusUnixFileDescriptor>
#include <QObject>
#include <functional>

namespace Coprocessor {

bool requestPowerOff();

// Holds a login1 shutdown delay lock, so the callback runs before the watch
// powers off or reboots.
class ShutdownWatcher : public QObject
{
    Q_OBJECT

public:
    using Callback = std::function<void()>;

    explicit ShutdownWatcher(Callback callback, QObject *parent = nullptr);

private slots:
    void onPrepareForShutdown(bool start);

private:
    void takeLock();

    Callback m_callback;
    QDBusUnixFileDescriptor m_lock;
};

}

#endif
