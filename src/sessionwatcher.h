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
#ifndef SESSIONWATCHER_H
#define SESSIONWATCHER_H

#include <QObject>
#include <QDBusObjectPath>
#include <QGSettings>
#include <QString>
#include <QTimer>

class SessionWatcher : public QObject
{
    Q_OBJECT
public:
    explicit SessionWatcher(QObject *parent = nullptr);

Q_SIGNALS:
    void sessionIdle();

private Q_SLOTS:
    void onStatusChanged(unsigned int status);
    void onSessionRemoved(const QDBusObjectPath &objectPath);
    void onConfigurationChanged(QString key);

private:
    QString sessionPath;
    QGSettings *settings;
    int idleDelay;
    QTimer *m_timer = nullptr;
};

#endif // SESSIONWATCHER_H
