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
#ifndef USERS_H
#define USERS_H

#include <QObject>


struct UserItem
{
    QString name;
    QString realName;
    QString icon;
    quint64 uid;
    QString path;   //accounts service path
    friend QDebug operator<<(QDebug stream, const UserItem &user);
};

class QDBusInterface;
class QDBusObjectPath;
class Users : public QObject
{
    Q_OBJECT
public:
    explicit Users(QObject *parent = nullptr);
    QList<UserItem> getUsers();
    UserItem getUserByName(const QString &name);
    QString getDefaultIcon();

private:
    void loadUsers();
    UserItem getUser(const QString &path);
    int findUserByPath(const QString &path);

private Q_SLOTS:
    void onUserAdded(const QDBusObjectPath& path);
    void onUserDeleted(const QDBusObjectPath& path);

Q_SIGNALS:
    void userAdded(const UserItem &user);
    void userDeleted(const UserItem &user);

private:
    QDBusInterface      *actService;
    QList<UserItem>     users;
    QString             defaultIcon;
};

#endif // USERS_H
