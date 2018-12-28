/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2017 Deepin, Inc.
 *               2011 ~ 2017 Wang Yong
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#ifndef XEVENTMONITOR_H
#define XEVENTMONITOR_H

#include <QThread>
#include <QMetaMethod>
#include <QDebug>

class XEventMonitorPrivate;
class XEventMonitor : public QThread
{
    Q_OBJECT

public:
    XEventMonitor(QObject *parent = 0);
    ~XEventMonitor();
    
Q_SIGNALS:
    void buttonPress(int x, int y);
    void buttonDrag(int x, int y);
    void buttonRelease(int x, int y);
    void keyPress(int keyCode);
    void keyRelease(int keyCode);
    void keyPress(const QString &key);
    void keyRelease(const QString &key);

protected:
    void run();
    
private:
    XEventMonitorPrivate *d_ptr;
    Q_DECLARE_PRIVATE(XEventMonitor)
};

#endif
