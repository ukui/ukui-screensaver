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

#include "sleeptime.h"
#include <QLabel>
#include <QDebug>
#include <QListWidget>

SleepTime::SleepTime(QWidget *parent) : QWidget(parent),
  sleepTime(0),
  localMin(0),
  timecount(100)
{
    init();
}

SleepTime::~SleepTime()
{

}

void SleepTime::init()
{
    layout = new QHBoxLayout(this);
    layout->setDirection(QBoxLayout::RightToLeft);
    layout->setSpacing(12);
    for(int i=0;i<2;i++)
    {
        QLabel *label = new QLabel(this);
        label->setText("0");
        label->setFixedSize(28,48);
        label->setObjectName("clockTime");
        list.append(label);
    }

    QLabel *colon = new QLabel(this);
    colon->setText(":");
    colon->setObjectName("colon");
    list.append(colon);

    for(int i=0;i<2;i++)
    {
        QLabel *label = new QLabel(this);
        label->setText("0");
        label->setFixedSize(28,48);
        label->setObjectName("clockTime");
        list.append(label);
    }

    for(int i=0;i<list.count();i++)
    {
        layout->addWidget(list.at(i));
    }

}

void SleepTime::setTime()
{
    sleepTime+=1;
    int sec = sleepTime % 60;
    int min = sleepTime/60;
    setSeconds(sec);
    if(min>localMin)
    {
        if(min>=timecount){
            timecount *=10;
            QLabel *label = new QLabel(this);
            label->setText("0");
            label->setFixedSize(28,48);
            label->setObjectName("clockTime");
            list.append(label);
            layout->addWidget(label);
        }

        localMin = min;
        setMinute(min);
    }
}

void SleepTime::setSeconds(int seconds)
{
    int sec1 = seconds%10;
    int sec2 = seconds/10;
    list.at(0)->setText(QString::number(sec1));
    if(sec1 == 0)
        list.at(1)->setText(QString::number(sec2));
}

void SleepTime::setMinute(int minutes)
{
    int min1 = minutes%10;
    int min2 = minutes/10;
    list.at(3)->setText(QString::number(min1));
    if(min1 == 0)
        list.at(4)->setText(QString::number(min2));
}
