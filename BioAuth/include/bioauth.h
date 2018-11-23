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
#ifndef BIOAUTH_H
#define BIOAUTH_H

#include <QObject>
#include "biotypes.h"

class QDBusInterface;

/*!
 * \brief The BioAuth class
 * 负责真正的生物识别操作，通过startAuth开始认证，
 * 认证完成后会发出携带结果的authComplete信号
 */
class BioAuth : public QObject
{
    Q_OBJECT
public:
    explicit BioAuth(qint32 uid, const DeviceInfo &deviceInfo, QObject *parent = nullptr);
    ~BioAuth();
    void setDevice(const DeviceInfo& deviceInfo);
    void startAuth();
    void stopAuth();
    bool isAuthenticating();

Q_SIGNALS:
    void authComplete(uid_t uid, bool result);
    void notify(const QString &message);

private Q_SLOTS:
    void onIdentityComplete(QDBusPendingCallWatcher *watcher);
    void onStatusChanged(int deviceId, int statusType);

private:
    QDBusInterface      *serviceInterface;

    qint32              uid;
    DeviceInfo          deviceInfo;
    bool isInAuthentication;
};

#endif // BIOAUTH_H
