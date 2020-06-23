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

#ifndef SLEEPTIME_H
#define SLEEPTIME_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QList>
#include <QHBoxLayout>

class SleepTime : public QWidget
{
    Q_OBJECT
public:
    explicit SleepTime(QWidget *parent = nullptr);
    ~SleepTime();
    int setTime();

private:
    QLabel *restTime;
    QList<QLabel *> list;
    QHBoxLayout *layout;
    int sleepTime;

    void init();
    void setSeconds(int seconds);
    void setMinute(int minutes);
};

#endif // SLEEPTIME_H
