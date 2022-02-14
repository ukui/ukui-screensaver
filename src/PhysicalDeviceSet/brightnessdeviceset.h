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

#ifndef BRIGHTNESSDEVICESET_H
#define BRIGHTNESSDEVICESET_H

#include <QObject>
#include <QMutex>
#include <QProcess>
#include <QDebug>

class BrightnessDeviceSet : public QObject
{
    Q_OBJECT
public:
    static BrightnessDeviceSet* instance(QObject *parent = nullptr);

public Q_SLOTS:
    // val from 0 to 100 change
    void setValue(int value);
    int getValue();

private:
    BrightnessDeviceSet(QObject *parent = nullptr);
    void init();
    bool isPowerSupply();
    int getMaxBrightness();
    int getCurBrightness();
    // val from 0 to MaxBrightness change
    void setBrightness(int val);

Q_SIGNALS:
    // val form  to 100 change
    //void valueChanged(int val);

private:
    static int m_value;
    static int m_maxBrightnessValue;
    static int m_curBrightnessValue;
    static bool m_isPowerSupply;
};

#endif // BRIGHTNESSDEVICESET_H
