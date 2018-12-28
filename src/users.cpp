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
#include "users.h"

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDebug>
#include <QFile>

#include "types.h"

QDebug operator<<(QDebug stream, const UserItem &user)
{
    stream << user.name << user.realName << user.uid
           << user.icon << user.path;
    return stream;
}

Users::Users(QObject *parent) : QObject(parent)
{
    defaultIcon = "/usr/share/pixmaps/faces/stock_person.png";
    loadUsers();
}

QList<UserItem> Users::getUsers()
{
    return users;
}

UserItem Users::getUserByName(const QString &name)
{
    auto iter = std::find_if(users.begin(), users.end(),
                          [&](const UserItem &user){
        return user.name == name;
    });
    return *iter;
}

QString Users::getDefaultIcon()
{
    return defaultIcon;
}


//https://stackoverflow.com/questions/20206376/
//how-do-i-extract-the-returned-data-from-qdbusmessage-in-a-qt-dbus-call
void Users::loadUsers()
{
    qDebug() << "loadUsers";
    actService = new QDBusInterface(ACT_DBUS_SERVICE,
                                    ACT_DBUS_PATH,
                                    ACT_DBUS_INTERFACE,
                                    QDBusConnection::systemBus());

    connect(actService, SIGNAL(UserAdded(const QDBusObjectPath&)),
            this, SLOT(onUserAdded(const QDBusObjectPath&)));
    connect(actService, SIGNAL(UserDeleted(const QDBusObjectPath&)),
            this, SLOT(onUserDeleted(const QDBusObjectPath&)));
    QDBusMessage ret = actService->call("ListCachedUsers");
    QList<QVariant> outArgs = ret.arguments();  //(QVariant(QDBusArgument,))
    QVariant first = outArgs.at(0); //QVariant(QDBusArgument,)
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
    QDBusObjectPath path;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd())
    {
        dbusArgs >> path;
        getUser(path.path());
    }
    dbusArgs.endArray();
}

UserItem Users::getUser(const QString &path)
{
    QDBusInterface iface(ACT_DBUS_SERVICE,
                         path,
                         DBUS_PROP_INTERFACE,
                         QDBusConnection::systemBus());
    QDBusMessage ret = iface.call("GetAll", ACT_USER_INTERFACE);
    QList<QVariant> outArgs = ret.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
    UserItem user;
    user.path = path;
    dbusArgs.beginMap();
    while(!dbusArgs.atEnd())
    {
        QString key;
        QVariant value;
        dbusArgs.beginMapEntry();
        dbusArgs >> key >> value;
        if(key == "UserName")
        {
            user.name = value.toString();
        }
        else if(key == "RealName")
        {
            user.realName = value.toString();
        }
        else if(key == "IconFile")
        {
            user.icon = value.toString();
            if(!QFile(user.icon).exists())
            {
                user.icon = defaultIcon;
            }
        }
        else if(key == "Uid")
        {
            user.uid = value.toUInt();
        }
        dbusArgs.endMapEntry();
    }
    dbusArgs.endMap();
    if(user.realName.isEmpty())
    {
        user.realName = user.name;
    }
    users.push_back(user);
    return user;
}

void Users::onUserAdded(const QDBusObjectPath& path)
{
    int index = findUserByPath(path.path());
    if(index != -1)
    {
        UserItem user = getUser(path.path());
        Q_EMIT userAdded(user);
    }
}
void Users::onUserDeleted(const QDBusObjectPath& path)
{
    int index = findUserByPath(path.path());
    if(index != -1)
    {
        UserItem user = users.at(index);
        users.removeAt(index);
        Q_EMIT userDeleted(user);
    }
}

int Users::findUserByPath(const QString &path)
{
    auto iter = std::find_if(users.begin(), users.end(),
                          [&](const UserItem &user){
        return user.path == path;
    });

    int index = iter - users.begin();

    return index;
}

