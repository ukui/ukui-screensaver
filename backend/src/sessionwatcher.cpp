#include "sessionwatcher.h"
#include <QDBusInterface>
#include <QDebug>
#include "types.h"

#define DBUS_SESSION_MANAGER_SERVICE "org.gnome.SessionManager"
#define DBUS_SESSION_MANAGER_PATH "/org/gnome/SessionManager/Presence"
#define DBUS_SESSION_MANAGER_INTERFACE "org.gnome.SessionManager.Presence"

SessionWatcher::SessionWatcher(QObject *parent) : QObject(parent)
{
    QDBusInterface *interface = new QDBusInterface(
                DBUS_SESSION_MANAGER_SERVICE,
                DBUS_SESSION_MANAGER_PATH,
                DBUS_SESSION_MANAGER_INTERFACE,
                QDBusConnection::sessionBus());

    connect(interface, SIGNAL(StatusChanged(unsigned int)),
            this, SLOT(onStatusChanged(unsigned int)));
}


void SessionWatcher::onStatusChanged(unsigned int status)
{
    qDebug() << "Session Status: " << status;

    if(status == ScreenSaver::SESSION_IDLE) {
        Q_EMIT sessionIdle();
    }
}
