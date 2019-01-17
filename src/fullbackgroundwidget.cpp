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
#include "fullbackgroundwidget.h"

#include <QGuiApplication>
#include <QScreen>
#include <QDBusInterface>
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QCloseEvent>

#include "lockwidget.h"
#include "xeventmonitor.h"
#include "monitorwatcher.h"
#include "configuration.h"
#include "screensaver.h"
#include "screensaverwidget.h"
#include "grab-x11.h"

FullBackgroundWidget::FullBackgroundWidget(QWidget *parent)
    : QWidget(parent),
      lockWidget(nullptr),
      xEventMonitor(new XEventMonitor(this)),
      monitorWatcher(new MonitorWatcher(this)),
      configuration(new Configuration(this)),
      isLocked(false),
      screenStatus(UNDEFINED)
{
    qDebug() << "init - screenStatus: " << screenStatus;

    connect(monitorWatcher, &MonitorWatcher::monitorCountChanged,
            this, &FullBackgroundWidget::onScreenCountChanged);
    QDesktopWidget *desktop = QApplication::desktop();
    connect(desktop, &QDesktopWidget::workAreaResized,
            this, &FullBackgroundWidget::onDesktopResized);
    connect(desktop, &QDesktopWidget::resized,
            this, &FullBackgroundWidget::onDesktopResized);

    QDBusInterface *iface = new QDBusInterface("org.freedesktop.login1",
                                               "/org/freedesktop/login1",
                                               "org.freedesktop.login1.Manager",
                                               QDBusConnection::systemBus(),
                                               this);
    connect(iface, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));

    init();
}

void FullBackgroundWidget::paintEvent(QPaintEvent *event)
{
    for(auto screen : QGuiApplication::screens())
    {
        QPainter painter(this);
        painter.drawPixmap(screen->geometry(), background);
    }
    return QWidget::paintEvent(event);
}

void FullBackgroundWidget::closeEvent(QCloseEvent *event)
{
    qDebug() << "FullBackgroundWidget::closeEvent";
    for(auto obj: children())
    {
        QWidget *widget = dynamic_cast<QWidget*>(obj);
        if(widget)
            widget->close();
    }
    closeGrab();
    return QWidget::closeEvent(event);
}

void FullBackgroundWidget::init()
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
                   /*| Qt::X11BypassWindowManagerHint*/);
//    setAttribute(Qt::WA_DeleteOnClose);
    establishGrab();

    // 监听session信号
    smInterface = new QDBusInterface(SM_DBUS_SERVICE,
                                     SM_DBUS_PATH,
                                     SM_DBUS_INTERFACE,
                                     QDBusConnection::sessionBus());
    connect(smInterface, SIGNAL(StatusChanged(uint)),
            this, SLOT(onSessionStatusChanged(uint)));

    connect(xEventMonitor, SIGNAL(keyPress(const QString &)),
            this, SLOT(onGlobalKeyPress(const QString &)));
    connect(xEventMonitor, SIGNAL(keyRelease(const QString &)),
            this, SLOT(onGlobalKeyRelease(const QString &)));
    connect(xEventMonitor, SIGNAL(buttonDrag(int, int)),
            this, SLOT(onGlobalButtonDrag(int, int)));

    int totalWidth = 0;
    int totalHeight = 0;
    for(auto screen : QGuiApplication::screens())
    {
        totalWidth += screen->geometry().width();
        totalHeight += screen->geometry().height();
    }
    setGeometry(0, 0, totalWidth, totalHeight);

    background.load(configuration->getBackground());

    xEventMonitor->start();
}

void FullBackgroundWidget::onCursorMoved(const QPoint &pos)
{
    if(!lockWidget)
    {
        return;
    }
    for(auto screen : QGuiApplication::screens())
    {
        if(screen->geometry().contains(pos))
        {
            lockWidget->setGeometry(screen->geometry());
            break;
        }
    }
}

void FullBackgroundWidget::lock()
{
    showLockWidget();

    lockWidget->startAuth();;
}


void FullBackgroundWidget::showLockWidget()
{
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    qDebug() << "showLockWidget - screenStatus: " << screenStatus;

    if(!lockWidget)
    {

        lockWidget = new LockWidget(this);
        connect(lockWidget, &LockWidget::closed,
                this, &FullBackgroundWidget::close);
        onCursorMoved(cursor().pos());
    }
    lockWidget->setFocus();
}

void FullBackgroundWidget::showScreensaver()
{
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_SAVER);
    qDebug() << "showScreensaver - screenStatus: " << screenStatus;

    for(auto screen : QGuiApplication::screens())
    {
        ScreenSaver *saver = configuration->getScreensaver();
        qDebug() << *saver;
        ScreenSaverWidget *saverWidget = new ScreenSaverWidget(saver, this);
        widgetXScreensaverList.push_back(saverWidget);
        saverWidget->setGeometry(screen->geometry());
    }
    setCursor(Qt::BlankCursor);

    //显示屏保时，停止认证（主要针对生物识别）
    if(lockWidget)
    {
        lockWidget->stopAuth();
    }
}

void FullBackgroundWidget::clearScreensavers()
{
    screenStatus = (ScreenStatus)(screenStatus & ~SCREEN_SAVER);

    for(auto widget : widgetXScreensaverList)
    {
        widget->close();
    }
    widgetXScreensaverList.clear();

    qDebug() << "clearScreensavers - screenStatus: " << screenStatus;


    unsetCursor();
    if(screenStatus == UNDEFINED)
    {
        close();
    }
    else
    {
        lock();
    }
}

void FullBackgroundWidget::onSessionStatusChanged(uint status)
{
    qDebug() << "session status changed: " << status;
    if(status != SESSION_IDLE)
    {
        //当前session没有处于空闲状态
        return;
    }
    qDebug() << "onSessionStatusChanged - screenStatus: " << screenStatus;

    if(screenStatus & SCREEN_SAVER)
    {
        return;
    }
    else if(screenStatus & SCREEN_LOCK)
    {
        showScreensaver();
    }
    else if(screenStatus == UNDEFINED)
    {
        if(configuration->xscreensaverActivatedWhenIdle() &&
                configuration->lockWhenXScreensaverActivated())
        {
            //显示锁屏和屏保
            showLockWidget();
            showScreensaver();
        }
        else if(configuration->xscreensaverActivatedWhenIdle())
        {
            //只显示屏保
            showScreensaver();
        }
    }
}

void FullBackgroundWidget::onGlobalKeyPress(const QString &key)
{

}

void FullBackgroundWidget::onGlobalKeyRelease(const QString &key)
{
    if(key == "Caps_Lock")
    {
        lockWidget->capsLockChanged();
    }
    else if(key == "Escape" && screenStatus == SCREEN_LOCK)
    {
        showScreensaver();
    }
    else if(screenStatus & SCREEN_SAVER)
    {
        clearScreensavers();
    }
}

void FullBackgroundWidget::onGlobalButtonDrag(int xPos, int yPos)
{
    if(screenStatus & SCREEN_SAVER)
    {
        clearScreensavers();
    }
}


void FullBackgroundWidget::onScreenCountChanged(int)
{
    QSize newSize = monitorWatcher->getVirtualSize();
    setGeometry(0, 0, newSize.width(), newSize.height());
    repaint();
    clearScreensavers();
}

void FullBackgroundWidget::onDesktopResized()
{
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->geometry());
    repaint();
    clearScreensavers();
}

void FullBackgroundWidget::onPrepareForSleep(bool sleep)
{
    ///系统休眠时，会关闭总线，导致设备不可用，发生错误
    ///在系统休眠之前停止认证，在系统唤醒后重新开始认证
    if(sleep)
    {
        lockWidget->stopAuth();
    }
    else
    {
        lockWidget->startAuth();
    }
}
