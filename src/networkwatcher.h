/**
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/

#ifndef NETWORKWATCHER_H
#define NETWORKWATCHER_H
#include <QObject>
#include <QtNetwork>
#include <QDBusInterface>

enum NMState{
    NM_STATE_UNKNOWN           = 0,
    NM_STATE_ASLEEP            = 10,
    NM_STATE_DISCONNECTED      = 20,
    NM_STATE_DISCONNECTING     = 30,
    NM_STATE_CONNECTING        = 40,
    NM_STATE_CONNECTED_LOCAL   = 50,
    NM_STATE_CONNECTED_SITE    = 60,
    NM_STATE_CONNECTED_GLOBAL  = 70,
    NM_STATE_CONNECTED_LIMIT   = 80
};

class NetWorkWatcher : public QObject
{
    Q_OBJECT

public:
    NetWorkWatcher(QObject *parent=nullptr);

Q_SIGNALS:
    void NetworkStateChanged(uint state);

private Q_SLOTS:
    void onNetworkStateChanged(uint state);
    void getIp(QHostInfo info);

public:
    void checkOnline();
    bool isConnect();

private:
    QDBusInterface *network;

};

#endif
