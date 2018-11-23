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
#include "bioauth.h"
#include <QList>

BioAuth::BioAuth(qint32 uid, const DeviceInfo &deviceInfo, QObject *parent)
    : QObject(parent),
      uid(uid),
      deviceInfo(deviceInfo),
      isInAuthentication(false)
{
    serviceInterface = new QDBusInterface(BIO_DBUS_SERVICE,
                                          BIO_DBUS_PATH,
                                          BIO_DBUS_INTERFACE,
                                          QDBusConnection::systemBus());

    connect(serviceInterface, SIGNAL(StatusChanged(int, int)),
            this, SLOT(onStatusChanged(int,int)));
    serviceInterface->setTimeout(2147483647);
}

BioAuth::~BioAuth()
{
    stopAuth();
}

void BioAuth::setDevice(const DeviceInfo &deviceInfo)
{
    this->deviceInfo = deviceInfo;
}

void BioAuth::startAuth()
{
    if(isInAuthentication)
        stopAuth();

    /* 开始认证识别 */
    LOG() << "start biometric verification";
    QList<QVariant> args;
    args << QVariant(deviceInfo.device_id) << QVariant(uid)
         << QVariant(0) << QVariant(-1);
    isInAuthentication = true;

    QDBusPendingCall call = serviceInterface->asyncCallWithArgumentList("Identify", args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &BioAuth::onIdentityComplete);
}


void BioAuth::stopAuth()
{
    QDBusReply<int> reply = serviceInterface->call("StopOps", QVariant(deviceInfo.device_id), QVariant(5));


    if(!reply.isValid())
        qWarning() << "StopOps error: " << reply.error();

    isInAuthentication = false;
}

bool BioAuth::isAuthenticating()
{
    return isInAuthentication;
}

void BioAuth::onIdentityComplete(QDBusPendingCallWatcher *watcher)
{


    QDBusPendingReply<qint32, qint32> reply = *watcher;
    if(reply.isError()){
        isInAuthentication = false;
        LOG() << reply.error();
        Q_EMIT authComplete(-1, false);
        return;
    }
    qint32 result = reply.argumentAt(0).toInt();
    qint32 retUid = reply.argumentAt(1).toInt();
    LOG() << "Identify complete: " << result << " " << retUid;

    /* 识别生物特征成功，发送认证结果 */
    if(isInAuthentication){

        isInAuthentication = false;

        if(result == DBUS_RESULT_SUCCESS && retUid == uid)
            Q_EMIT authComplete(retUid, true);
        else
            Q_EMIT authComplete(retUid, false);
    }
}


void BioAuth::onStatusChanged(int deviceId, int statusType)
{
    if(statusType != STATUS_NOTIFY)
        return;
    LOG() << "status changed " << deviceId << " " << statusType;
    QDBusMessage msg = serviceInterface->call("GetNotifyMesg", QVariant(deviceId));
    if(msg.type() == QDBusMessage::ErrorMessage){
        LOG() << msg.errorMessage();
        return;
    }
    QString message = msg.arguments().at(0).toString();
    LOG() << message;
    Q_EMIT notify(message);
}

