/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <QMimeType>
#include <QSettings>
#include <QMimeDatabase>
#include <QFileInfo>
#include <QFontMetrics>
#include "commonfunc.h"

bool ispicture(QString filepath)
{
	QFileInfo file(filepath);
	if(file.exists() == false)
		return false;

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filepath);
    return mime.name().startsWith("image/");
}

QString getSystemVersion()
{
    QSettings settings("/etc/lsb-release", QSettings::IniFormat);
    QString release = settings.value("DISTRIB_RELEASE").toString();
    QString description = settings.value("DISTRIB_DESCRIPTION").toString();
    if(description.right(3) == "LTS")
        release = release + " LTS";
    return release;
}

QString getSystemDistrib()
{
    QSettings settings("/etc/lsb-release", QSettings::IniFormat);
    QString distribId = settings.value("DISTRIB_ID").toString();
    return distribId;
}

bool getUseFirstDevice()
{
    QSettings settings("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
    return settings.value("UseFirstDevice").toBool();
}

commonFunc::commonFunc()
{

}

QString ElideText(QFont font,int width,QString strInfo)
{
    QFontMetrics fontMetrics(font);
    //如果当前字体下，字符串长度大于指定宽度
    if(fontMetrics.width(strInfo) > width)
    {
        strInfo= QFontMetrics(font).elidedText(strInfo, Qt::ElideRight, width);
    }
    return strInfo;
}

