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
#ifndef LOGIND_H
#define LOGIND_H

#include <QDBusConnection>
#include <QObject>

class QDBusServiceWatcher;

class LogindIntegration : public QObject
{
    Q_OBJECT
public:
    explicit LogindIntegration(QObject *parent = nullptr);
    ~LogindIntegration() override;

Q_SIGNALS:
    void requestLock();
    void requestUnlock();
    void connectedChanged();

private:

};

#endif
