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
#include <QDBusInterface>
#include <QTimer>
#include <unistd.h>
#include <QDBusPendingReply>
#include <QGSettings>
#include <signal.h>

Interface::Interface(QObject *parent)
    : QObject(parent),
      m_timerCount(0),
      settings(nullptr),
      m_timer(nullptr)
{
    lockState = false;
    m_logind = new LogindIntegration(this);
    connect(m_logind, &LogindIntegration::requestLock, this,
        [this]() {
            this->onShowScreensaver();
        }
    );
    connect(m_logind, &LogindIntegration::requestUnlock, this,
        [this]() {
        //process.terminate();
        process.kill();
	}
    );

    connect(&process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus exitStatus){
            emitLockState(false);
    });
	
    settings = new QGSettings("org.ukui.screensaver","",this);

    QDBusInterface *iface = new QDBusInterface("org.freedesktop.login1",
                                               "/org/freedesktop/login1",
                                               "org.freedesktop.login1.Manager",
                                               QDBusConnection::systemBus(),
                                               this);
    connect(iface, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));
    inhibit();

}

bool Interface::GetLockState()
{
	return ((process.state() != QProcess::NotRunning) && lockState);
}

void Interface::SetLockState()
{
    lockState = true;
}

void Interface::emitLockState(bool val)
{
    QDBusMessage message;
    if(val){
    	message = QDBusMessage::createSignal(SS_DBUS_PATH,
                                             SS_DBUS_INTERFACE,
                                             "lock");
    }else{
         message = QDBusMessage::createSignal(SS_DBUS_PATH,
                                             SS_DBUS_INTERFACE,
                                             "unlock");
    }
    QDBusConnection::sessionBus().send(message);
}

void Interface::Lock()
{
    if(process.state() != QProcess::NotRunning)
        return ;
    qDebug() << "Lock requested";
    lockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --lock");
    qDebug() << cmd;

    process.start(cmd);
    emitLockState(true);
}

void Interface::onSessionIdleReceived()
{
    if(process.state() != QProcess::NotRunning)
        return ;

    qDebug() << "emit SessionIdle";
    lockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --session-idle");
    qDebug() << cmd;
    process.start(cmd);
    emitLockState(true);
}

void Interface::onShowBlankScreensaver()
{
    if(process.state() != QProcess::NotRunning)
        return ;

    qDebug() << "lock and show screensaver";
    lockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --blank");
    qDebug() << cmd;

    process.start(cmd);
    emitLockState(true);
}

void Interface::onShowScreensaver()
{
    if(process.state() != QProcess::NotRunning)
        return ;

    qDebug() << "lock and show screensaver";
    lockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --screensaver");
    qDebug() << cmd;

    process.start(cmd);
    emitLockState(true);
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

void Interface::onPrepareForSleep(bool sleep)
{
    if(!settings->get("sleep-activation-enabled").toBool()){
   	uninhibit();
        return; 
    }
    
    if(sleep)
    {
        if(GetLockState()){
            uninhibit();
            return;
        }

    	this->onShowBlankScreensaver();

	if(!m_timer){
            m_timer = new QTimer(this);
            connect(m_timer, &QTimer::timeout, this, [&]{
                m_timerCount+=1;

                if(GetLockState() || m_timerCount>20){
                    m_timer->stop();
                    m_timerCount = 0;
                    uninhibit();
                }
            });
        }
        m_timer->start(100);
    }
    else
    {
        inhibit();
    }
}

void Interface::inhibit()
{
    if (m_inhibitFileDescriptor.isValid()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                                          "/org/freedesktop/login1",
                                                          "org.freedesktop.login1.Manager",
                                                          QStringLiteral("Inhibit"));
    message.setArguments(QVariantList({QStringLiteral("sleep"),
                                       "Screen Locker Backend",
                                       "Ensuring that the screen gets locked before going to sleep",
                                       QStringLiteral("delay")}));
    QDBusPendingReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(message);
    if (!reply.isValid()) {
        return;
    }
    reply.value().swap(m_inhibitFileDescriptor);
}

void Interface::uninhibit()
{
    if (!m_inhibitFileDescriptor.isValid()) {
        return;
    }

     m_inhibitFileDescriptor = QDBusUnixFileDescriptor();
}
