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

#define GSETTINGS_SCHEMA_SCREENSAVER "org.ukui.screensaver"
#define KEY_IDLE_DELAY "idleDelay"

SessionWatcher::SessionWatcher(QObject *parent) : QObject(parent)
{
    sessionPath = qgetenv("XDG_SESSION_PATH");

    QDBusInterface *interface = new QDBusInterface(
                SM_DBUS_SERVICE,
                SM_DBUS_PATH,
                SM_DBUS_INTERFACE,
                QDBusConnection::sessionBus());

    connect(interface, SIGNAL(StatusChanged(unsigned int)),
            this, SLOT(onStatusChanged(unsigned int)));

    QDBusInterface *displayManagerInterface = new QDBusInterface(
                DM_DBUS_SERVICE,
                DM_DBUS_PATH,
                DM_DBUS_INTERFACE,
                QDBusConnection::systemBus());
    connect(displayManagerInterface, SIGNAL(SessionRemoved(QDBusObjectPath)),
            this, SLOT(onSessionRemoved(QDBusObjectPath)));

    settings = new QGSettings(GSETTINGS_SCHEMA_SCREENSAVER, "", this);
    connect(settings, &QGSettings::changed,
            this, &SessionWatcher::onConfigurationChanged);
    idleDelay = settings->get("idle-delay").toInt();
}
 
void SessionWatcher::onConfigurationChanged(QString key)
{
    if(key == KEY_IDLE_DELAY){
        idleDelay = settings->get("idle-delay").toInt();
    }
}

void SessionWatcher::onStatusChanged(unsigned int status)
{
    qDebug() << "Session Status: " << status;

    if(status == SESSION_IDLE) {
        if(!m_timer){
            m_timer = new QTimer(this);
            connect(m_timer, &QTimer::timeout, this, [&]{
                Q_EMIT sessionIdle();
                m_timer->stop();
            });
        }
        int time = (idleDelay - 1)*60000;
        if(time<0)
            time = 0;
        m_timer->start(time);
    }else if(status == SESSION_AVAILABLE){
        if(m_timer && m_timer->isActive()){
            m_timer->stop();
        }
    }
}

void SessionWatcher::onSessionRemoved(const QDBusObjectPath &objectPath)
{
    //如果session注销了，则结束进程
    if(objectPath.path() == sessionPath)
        exit(0);
}
