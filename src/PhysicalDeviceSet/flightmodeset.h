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

#ifndef FLIGHTMODESET_H
#define FLIGHTMODESET_H
/*!
 *@brief 设置飞行模式
 * 本想在登录界面加载时 提供实体键 飞行模式的功能
 * 后来发现有效。但是飞行模式具体设置是谁负责管理的 还未知。
 * 最初实现的策略是通过 nmcli networking off 的方式设置飞行模式。
 */


#include <QObject>
#include <QMutex>

class FlightModeSet : public QObject
{
    Q_OBJECT
public:
    static FlightModeSet* instance(QObject *parent = nullptr);

private:
    FlightModeSet(QObject *parent = nullptr);

private:
    static bool m_isFlightModeOpen;

};

#endif // FLIGHTMODESET_H
