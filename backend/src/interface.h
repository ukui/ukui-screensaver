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
#ifndef INTERFACE_H
#define INTERFACE_H

#include <QObject>
#include <QDBusContext>
#include <QProcess>


class Interface : public QObject, protected QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "cn.kylinos.ScreenSaver")

public:
    explicit Interface(QObject *parent = nullptr);

Q_SIGNALS:
    void SessionIdle();

public Q_SLOTS:
    /**
     * Lock the screen
     */
    void Lock();

    void onSessionIdleReceived();

    void onNameLost(const QString&);

private:
    void runLocker(bool sessionIdle);
    bool checkExistChild();

private:
    QProcess process;
};

#endif // INTERFACE_H
