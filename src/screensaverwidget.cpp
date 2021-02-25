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
#include "screensaverwidget.h"
#include <unistd.h>
#include <signal.h>
#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <QPainter>
#include <QKeyEvent>
#include <QtX11Extras/QX11Info>
#include <X11/Xlib.h>
#include <sys/prctl.h>
ScreenSaverWidget::ScreenSaverWidget(ScreenSaver *screensaver, QWidget *parent)
    : QWidget(parent),
      timer(nullptr),
      screensaver(screensaver),
      closing(false)
{
    qDebug() << "ScreenSaverWidget::ScreenSaverWidget";
    setMouseTracking(true);
    setFocus();
    this->installEventFilter(this);

    QPalette plt;
    plt.setBrush(QPalette::Window, Qt::black);
    setPalette(plt);
    setAutoFillBackground(true);
    switch(screensaver->mode) {
    case SAVER_RANDOM:
    case SAVER_SINGLE:
        embedXScreensaver(screensaver->path);
        break;
    case SAVER_BLANK_ONLY:
        break;
    case SAVER_IMAGE:
    {
        setAutoFillBackground(true);
        screensaver->startSwitchImages();

        QPalette plt;
        plt.setBrush(QPalette::Background, Qt::black);
        setPalette(plt);

        connect(screensaver, &ScreenSaver::imagePathChanged,
                this, &ScreenSaverWidget::onBackgroundChanged);
        break;
    }
    case SAVER_DEFAULE:
        embedXScreensaver(screensaver->path);
        break;
    }
    show();
}

ScreenSaverWidget::~ScreenSaverWidget()
{

}

void ScreenSaverWidget::closeEvent(QCloseEvent *event)
{
    qDebug() << "ScreenSaverWidget::closeEvent---beginStop";
   if(process.state() != QProcess::NotRunning)
       process.kill();

    if(!closing){
        closing = true;
        screensaver->stopSwitchImages();
        delete screensaver;
        if(timer && timer->isActive())
            timer->stop();
    }
    qDebug() << "ScreenSaverWidget::closeEvent---endStop";
    return QWidget::closeEvent(event);
}

void ScreenSaverWidget::paintEvent(QPaintEvent *event)
{
    if(!screensaver->exists())
    {
        QPainter painter(this);
        painter.fillRect(geometry(), Qt::black);
    }
    if(screensaver->mode == SAVER_IMAGE) {
        switch(screensaver->effect) {
        case TRANSITION_NONE:
        {
            QPixmap pixmap(screensaver->path);
            pixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QPainter painter(this);
            painter.drawPixmap(geometry(), pixmap);
            break;
        }
        case TRANSITION_FADE_IN_OUT:
        {
            QPainter painter(this);
            QPixmap pixmap1(screensaver->lastPath);
            pixmap1.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            painter.setOpacity(opacity);
            painter.drawPixmap(geometry(), pixmap1);

            QPixmap pixmap(screensaver->path);
            pixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            painter.setOpacity(1 - opacity);
            painter.drawPixmap(geometry(), pixmap);
            break;
        }
        }


    }
    return QWidget::paintEvent(event);
}

bool ScreenSaverWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == 23)
    {
        XSetInputFocus(QX11Info::display(),this->winId(),RevertToParent,CurrentTime);
    }
    return false;
}

/* Embed xscreensavers */
void ScreenSaverWidget::embedXScreensaver(const QString &path)
{
    QString cmd = path + " -window-id " + QString::number(winId());
    if(process.state() == QProcess::NotRunning)
        process.start(cmd);
}



void ScreenSaverWidget::onBackgroundChanged(const QString &/*path*/)
{
    switch(screensaver->effect) {
    case TRANSITION_NONE:
        repaint();
        break;
    case TRANSITION_FADE_IN_OUT:
        opacity = 1.0;
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [&]{
            opacity -= 0.1;
            if(opacity <= 0)
                timer->stop();
            else
                repaint();

        });
        timer->start(50);
        break;
    }
}

