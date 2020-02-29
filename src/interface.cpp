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
#include <signal.h>

Interface::Interface(QObject *parent)
    : QObject(parent)
{
    m_logind = new LogindIntegration(this);
    connect(m_logind, &LogindIntegration::requestLock, this,
        [this]() {
            this->onSessionIdleReceived();
        }
    );
    connect(m_logind, &LogindIntegration::requestUnlock, this,
        [this]() {

                char cmd[228] = {0};
                char str[16];
                FILE *fp;
                int pid;

                int n = sprintf(cmd, "ps -o ruser=abcdefghijklmnopqrstuvwxyz1234567890 -e -o pid,stime,cmd| grep ukui-screensaver-dialog | grep %s | grep -v grep | awk '{print $2}'", getenv("USER"));
                Q_UNUSED(n)

                fp = popen(cmd, "r");
                while(fgets(str, sizeof(str)-1, fp)) {
                        pid = atoi(str);
                        if(pid > 0 && pid != getpid()) {
                                kill(pid, SIGKILL);
                        }
                }
                pclose(fp);
        }
    );
}

void Interface::Lock()
{
    qDebug() << "Lock requested";

    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --lock");
    qDebug() << cmd;

    process.startDetached(cmd);

}

void Interface::onSessionIdleReceived()
{
    qDebug() << "emit SessionIdle";

    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --session-idle");
    qDebug() << cmd;

    process.startDetached(cmd);

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
