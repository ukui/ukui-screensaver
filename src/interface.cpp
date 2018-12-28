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
#include "interface.h"
#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <unistd.h>

Interface::Interface(QObject *parent)
    : QObject(parent)
{

}

void Interface::Lock()
{
    qDebug() << "Lock requested";

    if(!checkExistChild())
    {
        QString cmd = QString("/usr/bin/ukui-screensaver-dialog --lock");
        qDebug() << cmd;

        process.startDetached(cmd);
    }
}

void Interface::onSessionIdleReceived()
{
    qDebug() << "emit SessionIdle";

    if(!checkExistChild())
    {
        QString cmd = QString("/usr/bin/ukui-screensaver-dialog --session-idle");
        qDebug() << cmd;

        process.startDetached(cmd);
    }
}

bool Interface::checkExistChild()
{
    char cmd[128] = {0};
    char str[4];
    FILE *fp;
    int num;

    sprintf(cmd, "ps -aux | grep ukui-screensaver-dialog | grep %s | grep -v grep | wc -l", getenv("USER"));

    fp = popen(cmd, "r");
    if(fgets(str, sizeof(str)-1, fp) == NULL)
        qDebug() << "fgets: " << strerror(errno);
    pclose(fp);

    num = atoi(str);

    qDebug() << (num > 0 ? "exist dialog running" : "");

    return num > 0;
}

void Interface::onNameLost(const QString &serviceName)
{
    if(serviceName == "cn.kylinos.ScreenSaver")
        exit(0);
}
