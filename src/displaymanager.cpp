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
#include "displaymanager.h"
#include <QDBusInterface>
#include <QDebug>
#include <QFile>
#include <QDBusMessage>
#include <QDBusArgument>
#include "types.h"

DisplayManager::DisplayManager(QObject *parent) :
    QObject(parent),
    _canSwitch(false),
    _displayType(""),
    process(nullptr)
{

    if(getenv("XDG_SEAT_PATH")){
        _displayType = "lightdm";
    }
    else if(process_is_running ("gdm") || process_is_running("gdm3") || process_is_running("gdm-binary")){
        _displayType = "gdm";
        process = new QProcess(this);
    }

    if(_displayType == "lightdm"){
        char *seatPath = getenv("XDG_SEAT_PATH");
        qDebug() << seatPath;

        dmService = new QDBusInterface(DM_DBUS_SERVICE,
                                   seatPath,
                                   DBUS_PROP_INTERFACE,
                                   QDBusConnection::systemBus());
        dmSeatService = new QDBusInterface(DM_DBUS_SERVICE,
                                       seatPath,
                                       DM_SEAT_INTERFACE,
                                       QDBusConnection::systemBus());
        getProperties();
    }else if(_displayType == "gdm"){
        _canSwitch = true;
    }
}

bool DisplayManager::canSwitch()
{
    return _canSwitch;
}

bool DisplayManager::hasGuestAccount()
{
    return _hasGuestAccount;
}

QString DisplayManager::getDisplayType()
{
    return _displayType;
}

void DisplayManager::switchToGreeter()
{
    qDebug()<<"111111111111111111111111111111111111111111111111111111";
    if(_displayType == "lightdm"){
        QDBusMessage ret = dmSeatService->call("SwitchToGreeter");
        handleDBusError(ret);
    }
    else if(_displayType == "gdm"){
        QString cmd = "gdmflexiserver";
        if(QFile("/usr/bin/gdmflexiserver").exists()){
            process->waitForFinished(3000);
             process->start(cmd);
        }
    }
}

void DisplayManager::switchToUser(const QString &userName)
{
    if(_displayType != "lightdm")
        return ;

    QDBusMessage ret = dmSeatService->call("SwitchToUser", userName, "");

    handleDBusError(ret);
}

void DisplayManager::switchToGuest()
{
    if(_displayType != "lightdm")
        return ;

    QDBusMessage ret = dmSeatService->call("SwitchToGuest", "");

    handleDBusError(ret);
}

void DisplayManager::getProperties()
{
    QDBusMessage ret = dmService->call("GetAll", DM_SEAT_INTERFACE);
    handleDBusError(ret);
    const QDBusArgument &arg = ret.arguments().at(0).value<QDBusArgument>();
    qDebug() << arg.currentType();

    arg.beginMap();
    while(!arg.atEnd())
    {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        arg.endMapEntry();
        if(key == "CanSwitch")
        {
            _canSwitch = value.toBool();
        }
        else if(key == "HasGuestAccount")
        {
            _hasGuestAccount = value.toBool();
        }
    }
    arg.endMap();
}

bool DisplayManager::process_is_running(QString name)
{
     int num_processes;
     QString command = "pidof " + name + " | wc -l";
     FILE *fp = popen(command.toLatin1().data(), "r");

     if (fscanf(fp, "%d", &num_processes) != 1)
        num_processes = 0;

     pclose(fp);

     if (num_processes > 0) {
            return true;
     } else {
            return false;
     }
}

void DisplayManager::handleDBusError(const QDBusMessage &msg)
{
    if(msg.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << msg.errorMessage();
    }
}
