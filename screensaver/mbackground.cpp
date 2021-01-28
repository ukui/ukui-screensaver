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

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QMimeDatabase>
#include <ctime>
#include "mbackground.h"

#include "commonfunc.h"

MBackground::MBackground():
    currentIndex(0)
{
    getAllPixmap();

}

QString MBackground::getCurrent()
{
    if(list.count()<0)
        return "";

    if(currentIndex>=0 && currentIndex<list.count())
        return list.at(currentIndex);

    return "";
}

QString MBackground::getNext()
{
    if(list.count() == 1)
        return list.at(0);

    if(list.count()<=0 || currentIndex>=list.count() ||currentIndex<0)
        return "";

    if(currentIndex == list.count() - 1)
        currentIndex = 0;
    else
        currentIndex++;

    return list.at(currentIndex);
}

QString MBackground::getPrev()
{
    if(list.count() == 1)
        return list.at(0);

    if(list.count()<=0 || currentIndex>=list.count() || currentIndex<0)
        return "";

    if(currentIndex == 0)
        currentIndex = list.count() - 1;
    else
        currentIndex--;

    return list.at(currentIndex);
}

QString MBackground::getRand()
{
    if(list.count() <= 0)
        return "";
    qsrand(time(NULL));
    currentIndex = qrand() % list.count();

    return list.at(currentIndex);
}

void MBackground::getAllPixmap()
{
    QDomDocument doc;
    QFile *file;
    file = new QFile("/usr/share/ukui-background-properties/focal-ubuntukylin-wallpapers.xml");
    if (!file->open(QIODevice::ReadOnly))
    {
        qDebug()<<file->fileName()<<" open failed";
        return ;
    }

    if (!doc.setContent(file))
    {
        file->close();
        return ;
    }
    file->close();
    file->deleteLater();
    QDomElement root = doc.documentElement();//读取根节点
    QDomNode node = root.firstChild();//读取第一个子节点   QDomNode 节点
    while (!node.isNull())
    {
        QDomElement node1 = node.firstChildElement("filename");
        QString fileName = node1.text();
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(fileName);
        if(mime.name().startsWith("image/")){
            list.append(fileName);
        }

        node = node.nextSibling();//读取下一个兄弟节点
    }
}
