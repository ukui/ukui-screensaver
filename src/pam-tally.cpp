/*
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
#include "pam-tally.h" 
#include <QFile>
#include <QStringList>
#include <QString>
#include <QIODevice>
#include <QDebug>
#define PAM_CONFIG_FILE "/etc/pam.d/common-auth"

PamTally* PamTally::instance_ = nullptr;

PamTally::PamTally(QObject *parent)
    : QObject(parent)
{
    parsePamConfig();
}

PamTally* PamTally::instance(QObject *parent)
{
    if(instance_ == nullptr)
        instance_ = new PamTally(parent);
    return instance_;
}

int PamTally::parsePamConfig()
{
    deny = 0;
    unlock_time = 0;
    root_unlock_time = 0;

    QFile file(PAM_CONFIG_FILE);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        qDebug()<<"open /etc/pam.d/common-auth failed";
        return -1;
    }

    QStringList list;
    list.clear();

    while(!file.atEnd()){
        QString line = (QString)file.readLine();
        line = line.trimmed();

        if(line.startsWith("#") || !line.contains("unix",Qt::CaseSensitive))
            continue;

        list << line;
    }
    file.close();

    if(list.size() > 0)
    {
        QString line = list.at(0);
        QStringList strs = line.split(" ");
        foreach (QString str, strs) {
            if(str.contains("deny",Qt::CaseSensitive))
            {
                deny = str.split("=").at(1).toUInt();
            }
            if(str.contains("unlock_time",Qt::CaseSensitive))
            {
                unlock_time = str.split("=").at(1).toULongLong();
            }
            if(str.contains("root_unlock_time",Qt::CaseSensitive))
            {
                root_unlock_time = str.split("=").at(1).toULongLong();
            }
        }
    }
    qDebug()<<"deny = "<<deny<<"unlock_time  = "<<unlock_time<<"root_unlock_time = "<<root_unlock_time;
    return 1;
}

int PamTally::getDeny()
{
    return deny;
}

unsigned long long PamTally::getRootUnlockTime()
{
    return root_unlock_time;
}

unsigned long long PamTally::getUnlockTime()
{
    return unlock_time;
}
