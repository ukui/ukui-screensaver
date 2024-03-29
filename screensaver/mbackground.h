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

#ifndef MBACKGROUND_H
#define MBACKGROUND_H

#include <QList>
#include <QString>
#include <QGSettings>
#include <QTimer>
#include <QStringList>
#include "scconfiguration.h"

class MBackground
{
public:
    MBackground();
    QString getCurrent();
    QString getNext();
    QString getPrev();
    QString getRand();

private:
    void getAllPixmap();

    QStringList list;
    int currentIndex;
};

#endif // MBACKGROUND_H
