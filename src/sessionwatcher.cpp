/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
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

    if(status == SESSION_IDLE) {
        Q_EMIT sessionIdle();
    }
}

void SessionWatcher::onSessionRemoved(const QDBusObjectPath &objectPath)
{
    //如果session注销了，则结束进程
    if(objectPath.path() == sessionPath)
        exit(0);
}
