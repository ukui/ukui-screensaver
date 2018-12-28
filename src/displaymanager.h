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
#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <QObject>

class QDBusInterface;
class QDBusMessage;

class DisplayManager : public QObject
{
    Q_OBJECT
public:
    explicit DisplayManager(QObject *parent = nullptr);
    void switchToGreeter();
    void switchToUser(const QString &userName);
    void switchToGuest();
    bool canSwitch();
    bool hasGuestAccount();

private:
    void getProperties();
    void handleDBusError(const QDBusMessage &msg);

private:
    bool _canSwitch;
    bool _hasGuestAccount;

    QDBusInterface *dmService;
    QDBusInterface *dmSeatService;
};

#endif // DISPLAYMANAGER_H
