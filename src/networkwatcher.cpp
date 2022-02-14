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

#include <QDBusReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDBusMessage>

#include "networkwatcher.h"

const int port = 80;

NetWorkWatcher::NetWorkWatcher(QObject *parent)
{
    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                         QString("/org/freedesktop/NetworkManager"),
                                         QString("org.freedesktop.NetworkManager"),
                                         QString("StateChanged"), this, SLOT(onNetworkStateChanged(uint)));

    network = new QDBusInterface("org.freedesktop.NetworkManager",
                                 "/org/freedesktop/NetworkManager",
                                 "org.freedesktop.NetworkManager",
                                 QDBusConnection::systemBus());
}

void NetWorkWatcher::onNetworkStateChanged(uint state)
{
    qDebug() << "NetWork state already changed to" << state;
    emit NetworkStateChanged(state);
}

//网络是否可以连接
void NetWorkWatcher::checkOnline()
{
    QHostInfo::lookupHost("www.baidu.com", this, SLOT(getIp(QHostInfo))); //异步非阻塞
    /*QHostInfo info = QHostInfo::fromName("www.baidu.com"); //阻塞
    if(info.error() != QHostInfo::NoError)
        return false;
    QString ipAddress = info.addresses().first().toString();

    QTcpSocket tcpClient;
    tcpClient.abort();
    tcpClient.connectToHost(ipAddress, port);
    //150毫秒没有连接上则判断不在线
    return tcpClient.waitForConnected(150);*/
}

//是否连接网络
bool NetWorkWatcher::isConnect()
{
    QDBusMessage message = network->call("state");
    if(QDBusMessage::ErrorMessage == message.type())
    {
        qDebug() << "error: NetWorkManager DBus Connect Failed!";
        return false;
    }

    QList<QVariant> argvs = message.arguments();
    uint state = argvs.at(0).value<uint>();

    qDebug() << "NetWorkManager state:" << state;

    if(state == NM_STATE_CONNECTED_SITE || state == NM_STATE_CONNECTED_LOCAL || state == NM_STATE_CONNECTED_GLOBAL)
        return true;

    return false;
}

void NetWorkWatcher::getIp(QHostInfo info)
{
    if(!isConnect())
    {
        emit NetworkStateChanged(NM_STATE_DISCONNECTED);
        return;
    }

    if(info.error() != QHostInfo::NoError)
    {
        emit NetworkStateChanged(NM_STATE_CONNECTED_LIMIT);
        return;
    }

    QString ipAddress = info.addresses().first().toString();

    QTcpSocket tcpClient;
    tcpClient.abort();
    tcpClient.connectToHost(ipAddress, port);
    //150毫秒没有连接上则判断不在线
    if(!tcpClient.waitForConnected(150))
    {
        emit NetworkStateChanged(NM_STATE_CONNECTED_LIMIT);
        return;
    }
    emit NetworkStateChanged(NM_STATE_CONNECTED_GLOBAL);
}

