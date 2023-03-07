#include "coprocessor.h"

#include "display_interface.h"

#include "dbus.h"

Coprocessor::Coprocessor(QObject *parent) : QObject(parent)
{
    display = new org::asteroid::display(SERVICE_NAME, DISPLAY_OBJECT, QDBusConnection::sessionBus(), this);
}

Coprocessor::~Coprocessor()
{
}

bool Coprocessor::hasCoprocessor() const
{
    if (!display->isValid()) {
        qDebug() << "No remote connection! Daemon not active?";
        return false;
    }

    return display->HasCoprocessor();
}
bool Coprocessor::synchronizeTime() const
{
    if (!display->isValid()) {
        qDebug() << "No remote connection! Daemon not active?";
        return false;
    }

    return display->SynchronizeTime();
}