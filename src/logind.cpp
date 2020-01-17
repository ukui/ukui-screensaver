/*
 * Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "logind.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

const static QString login1Service = QStringLiteral("org.freedesktop.login1");
const static QString login1Path = QStringLiteral("/org/freedesktop/login1");
const static QString login1ManagerInterface = QStringLiteral("org.freedesktop.login1.Manager");
const static QString login1SessionInterface = QStringLiteral("org.freedesktop.login1.Session");

LogindIntegration::LogindIntegration(QObject *parent)
    : QObject(parent)
{
    QDBusInterface loginInterface(login1Service,
                             login1Path,
                             login1ManagerInterface,
                             QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> sessionPath = loginInterface.call("GetSessionByPID",(quint32) QCoreApplication::applicationPid());
    if(!sessionPath.isValid()){
        qWarning()<< "Get session error:" << sessionPath.error();
    }
    else{

        QString session = sessionPath.value().path();
        QDBusConnection::systemBus().connect(login1Service,
                      session,
                      login1SessionInterface,
                      QStringLiteral("Lock"),
                      this,
                      SIGNAL(requestLock()));
        QDBusConnection::systemBus().connect(login1Service,
                      session,
                      login1SessionInterface,
                      QStringLiteral("Unlock"),
                      this,
                      SIGNAL(requestUnlock()));
    }
    return;
}

LogindIntegration::~LogindIntegration() = default;

