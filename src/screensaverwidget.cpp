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
#include "configuration.h"
#include <unistd.h>
#include <signal.h>
#include <QDebug>
#include <QGuiApplication>
#include <QTimer>
#include <QLabel>
#include <QScreen>
#include <QDate>
#include <QVBoxLayout>
#include <QDateTime>
#include <QPainter>

ScreenSaverWidget::ScreenSaverWidget(ScreenSaver *screensaver, QWidget *parent)
    : QWidget(parent),
      timer(nullptr),
      xscreensaverPid(-1),
      screensaver(screensaver),
      closing(false)
{
    qDebug() << "ScreenSaverWidget::ScreenSaverWidget";
    qDebug() << *screensaver;
    setMouseTracking(true);
    setFocus();

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
    case SAVER_DEFAULT:
    	initUI();
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
    if(xscreensaverPid > 0)
        kill(xscreensaverPid, SIGKILL);

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

QImage ScreenSaverWidget::Bright1(QImage &image,int brightness)
{
    uchar *line =image.scanLine(0);
    uchar *pixel = line;

    for (int y = 0; y < image.height(); ++y)
    {
        pixel = line;
        for (int x = 0; x < image.width(); ++x)
        {
            *pixel = qBound(0, *pixel + brightness, 255);
            *(pixel + 1) = qBound(0, *(pixel + 1) + brightness, 255);
            *(pixel + 2) = qBound(0, *(pixel + 2) + brightness, 255);
            pixel += 4;
        }

        line += image.bytesPerLine();
    }
    return image;

}

void ScreenSaverWidget::paintEvent(QPaintEvent *event)
{
    if(!screensaver->exists())
    {
        QPainter painter(this);
        painter.fillRect(geometry(), Qt::black);
    }
    if(screensaver->mode == SAVER_DEFAULT)
    {
        QPixmap pixmap(screensaver->path);
            QImage tempImage = pixmap.toImage();
            tempImage = Bright1(tempImage,-80);
            pixmap =  QPixmap::fromImage(tempImage);
        pixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QPainter painter(this);

        painter.setOpacity(0.8);
        painter.drawPixmap(geometry(), pixmap);
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


/* Embed xscreensavers */
void ScreenSaverWidget::embedXScreensaver(const QString &path)
{
    unsigned long wid = winId();
    char widStr[20] = {0};
    snprintf(widStr, sizeof(widStr), "%lu", wid);
    if((xscreensaverPid = fork()) == 0) {
        execl(path.toStdString().c_str(), "xscreensaver", "-window-id", widStr, (char*)0);
        qWarning() << "exec " << path << "failed";
    }
    qDebug() << "xscreensaver pid: " << xscreensaverPid;
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

QString ScreenSaverWidget::getlocktime()
{

    QString ssec,sminu,shour;
    QString res;
    if(sec>=60)
    {
        minu+=1;
        sec-=60;
    }
    if(minu>=60)
    {
        hour+=1;
        minu-=60;
    }

    if(sec<10){
        ssec.setNum(sec);
        ssec = "0" + ssec;
    }
    else {
        ssec.setNum(sec);
    }

    if(minu<10){
        sminu.setNum(minu);
        sminu = "0" + sminu;
    }
    else {
        sminu.setNum(minu);
    }

    if(hour<10){
        shour.setNum(hour);
        shour = "0" + shour;
    }
    else {
        shour.setNum(hour);
    }
    res = tr("You have rested") + ":" + shour + ":" + sminu + ":"+ssec;
    return res;
}

void ScreenSaverWidget::initUI()
{
    for(auto screen : QGuiApplication::screens())
    {
            this->setGeometry(screen->geometry());

    }


    setAutoFillBackground(true);
    QPalette pal = this->palette();
    pal.setBrush(QPalette::Background, Qt::black);
    setPalette(pal);

    timer = new QTimer(this);
    lblTime = new QLabel (this);
    lblDate = new QLabel (this);
    lblWeek = new QLabel (this);
    lbllocktime = new QLabel (this);
    connect(timer, &QTimer::timeout, this, [&]{
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        lblTime->setText(time);
        QString date = QDate::currentDate().toString("yyyy/MM/dd dddd");
        QStringList datelist = date.split(" ");
        lblDate->setText(datelist.at(0));
        lblWeek->setText(datelist.at(1));
    });

    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    lblTime->setText(time);
    lblTime->setStyleSheet("QLabel{color:white; font-size: 70px;}");
    lblTime->adjustSize();
    int y = this->geometry().height()/8 + 60;
    int x = (this->geometry().width()-lblTime->width())/2;
    lblTime->setGeometry(x,y,lblTime->width(),lblTime->height());
    timer->start(1000);

    QString date = QDate::currentDate().toString("yyyy/MM/dd dddd");
    QStringList datelist = date.split(" ");
    lblDate->setText(datelist.at(0)) ;
    lblDate->setStyleSheet("QLabel{color:white; font-size: 30px;}");
    lblDate->adjustSize();
    y = this->geometry().height()/8 + 170;
    x = (this->geometry().width()-lblDate->width())/2;
    lblDate->setGeometry(x,y,lblDate->width(),lblDate->height());

    lblWeek->setText(datelist.at(1));
    lblWeek->setStyleSheet("QLabel{color:white; font-size: 40px;}");
    lblWeek->adjustSize();
    y = this->geometry().height()/8;
    x = (this->geometry().width()-lblWeek->width())/2;
    lblWeek->setGeometry(x,y,lblWeek->width(),lblWeek->height());

    label = new QLabel(this);
    label->setText("因为有梦，所以披星戴月\n因为有梦，所以奋不顾身");
    label->setStyleSheet("QLabel{color:white; font-size: 55px;}");
    label->adjustSize();
    y = this->geometry().height()/6 + 240;
    x = (this->geometry().width()-label->width())/2;
    label->setGeometry(x,y,label->width(),label->height());

     tim = new QTimer(this);
     qDebug()<<"sec = "<<sec;
    connect(tim, &QTimer::timeout, this, [&]{
        QString locktime = getlocktime();
        lbllocktime->setText(locktime);
        sec += 1;
    });
    tim->start(1000);
    QString locktime = getlocktime();
    lbllocktime->setText(locktime);
    lbllocktime->setStyleSheet("QLabel{color:white; font-size: 40px;}");
    lbllocktime->adjustSize();
    y = this->geometry().height()-lbllocktime->height()-50;
    x = this->geometry().width()-lbllocktime->width() - 50;

    lbllocktime->setGeometry(x,y,lbllocktime->width(),lbllocktime->height());


}
