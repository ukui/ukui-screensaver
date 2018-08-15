#include "sessionwatcher.h"
#include <QDBusInterface>
#include <QDebug>
#include "types.h"

#define DBUS_SESSION_MANAGER_SERVICE "org.gnome.SessionManager"
#define DBUS_SESSION_MANAGER_PATH "/org/gnome/SessionManager/Presence"
#define DBUS_SESSION_MANAGER_INTERFACE "org.gnome.SessionManager.Presence"

#define DBUS_DISPLAY_MANAGER_SERVICE "org.freedesktop.DisplayManager"
#define DBUS_DISPLAY_MANAGER_PATH "/org/freedesktop/DisplayManager"
#define DBUS_DISPLAY_MANAGER_INTERFACE "org.freedesktop.DisplayManager"

SessionWatcher::SessionWatcher(QObject *parent) : QObject(parent)
{
    sessionPath = qgetenv("XDG_SESSION_PATH");

    QDBusInterface *interface = new QDBusInterface(
                DBUS_SESSION_MANAGER_SERVICE,
                DBUS_SESSION_MANAGER_PATH,
                DBUS_SESSION_MANAGER_INTERFACE,
                QDBusConnection::sessionBus());

    connect(interface, SIGNAL(StatusChanged(unsigned int)),
            this, SLOT(onStatusChanged(unsigned int)));

    QDBusInterface *displayManagerInterface = new QDBusInterface(
                DBUS_DISPLAY_MANAGER_SERVICE,
                DBUS_DISPLAY_MANAGER_PATH,
                DBUS_DISPLAY_MANAGER_INTERFACE,
                QDBusConnection::systemBus());
    connect(displayManagerInterface, SIGNAL(SessionRemoved(QDBusObjectPath)),
            this, SLOT(onSessionRemoved(QDBusObjectPath)));
}


void SessionWatcher::onStatusChanged(unsigned int status)
{
    qDebug() << "Session Status: " << status;

    if(status == ScreenSaver::SESSION_IDLE) {
        Q_EMIT sessionIdle();
    }
}

void SessionWatcher::onSessionRemoved(const QDBusObjectPath &objectPath)
{
    //如果session注销了，则结束进程
    if(objectPath.path() == sessionPath)
        exit(0);
}
