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
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>

#include "interface.h"
#include "sessionwatcher.h"
#include "screensaveradaptor.h"
#include "types.h"

#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>


void sig_chld(int /*signo*/)
{
    pid_t pid;
    while( (pid = waitpid(-1, NULL, WNOHANG)) > 0)
        qDebug() << "child" << pid << "terminated";
    return;
}

int main(int argc, char *argv[])
{
//    if(signal(SIGCHLD, sig_chld) == SIG_ERR) {
//        perror("signal error");
//        exit(EXIT_FAILURE);
//    }

    QCoreApplication a(argc, argv);

    // 检查该程序是否已经有实例在运行
    QDBusInterface *checkInterface =
            new QDBusInterface("org.freedesktop.DBus",
                               "/org/freedesktop/DBus",
                               "org.freedesktop.DBus",
                               QDBusConnection::sessionBus());
    QDBusReply<bool> ret = checkInterface->call("NameHasOwner",
                                               "cn.kylinos.ScreenSaver");
    if(ret.value()) {
        qDebug() << "There is an instance running";
        exit(EXIT_FAILURE);
    }

    // 如果已经有实例在运行则kill, 主要是针对注销后重新登录时之前的实例没有被kill掉
    char cmd[128] = {0};
    char str[16];
    FILE *fp;
    int pid;

    int n = sprintf(cmd, "ps aux | grep ukui-screensaver-backend | grep %s | grep -v grep | awk '{print $2}'", getenv("USER"));
    Q_UNUSED(n)

    fp = popen(cmd, "r");
    while(fgets(str, sizeof(str)-1, fp)) {
        pid = atoi(str);

        if(pid > 0 && pid != getpid()) {
            qDebug() << "existing instance pid: " << pid;
            kill(pid, SIGKILL);
        }
    }
    pclose(fp);

    // for PowerManager
    fp = popen("xset s 0 0", "r");
    fclose(fp);
//    Q_UNUSED(fp)


    // 注册DBus
    Interface *interface = new Interface();
    ScreenSaverAdaptor adaptor(interface);

    QDBusConnection service = QDBusConnection::sessionBus();
    if(!service.registerService(SS_DBUS_SERVICE)) {
        qDebug() << service.lastError().message();
        exit(EXIT_FAILURE);
    }
    if(!service.registerObject(SS_DBUS_PATH, SS_DBUS_SERVICE, &adaptor,
                               QDBusConnection::ExportAllSlots |
                               QDBusConnection::ExportAllSignals)) {
        qDebug() << service.lastError().message();
        exit(EXIT_FAILURE);
    }
    qDebug() << service.baseService();

    // 发送DBus信号
    SessionWatcher *watcher = new SessionWatcher;
    QObject::connect(watcher, &SessionWatcher::sessionIdle,
                     interface, &Interface::onSessionIdleReceived);
    QObject::connect(watcher, &SessionWatcher::sessionIdle,
                     &a, [&]{
        QDBusMessage message = QDBusMessage::createSignal(SS_DBUS_PATH,
                                                          SS_DBUS_INTERFACE,
                                                          "SessionIdle");
        service.send(message);
    });


    QObject::connect(checkInterface, SIGNAL(NameLost(QString)),
                     interface, SLOT(onNameLost(QString)));


    return a.exec();
}
