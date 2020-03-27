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

#include <QWidget>
#include <QPalette>
#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QDate>
#include <QKeyEvent>
#include <QDateTime>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QX11Info>
#include <QApplication>
#include "screensaver.h"

Screensaver::Screensaver(QWidget *parent):
  QWidget(parent),
  date(new ChineseDate()),
  centerWidget(nullptr),
  dateOfLunar(nullptr)
{
    installEventFilter(this);
    initUI();

    settings = new QGSettings("org.mate.background","",this);
    background = settings->get("picture-filename").toString();
    QList<QLabel*> labelList = this->findChildren<QLabel *>();
    for(int i = 0;i<labelList.count();i++)
    {
        labelList.at(i)->setAlignment(Qt::AlignCenter);
    }
}

Screensaver::~Screensaver()
{

}

bool Screensaver::eventFilter(QObject *obj, QEvent *event)
{

    if(event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() ==Qt::Key_Q || keyEvent->key() == Qt::Key_Escape){
            qApp->quit(); //需要 #include <QApplication> 头文件
        }
    }
    return false;
}

void Screensaver::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
    QPixmap pixmap(background);
	painter.drawPixmap(0,0,this->width(),this->height(),pixmap);
    painter.setBrush(QColor(0,0,0,200));
    painter.drawRect(0,0,this->width(),this->height());

}

void Screensaver::resizeEvent(QResizeEvent */*event*/)
{
    int x = (this->width()-timeLayout->geometry().width())/2;
    int y = 50;
    timeLayout->setGeometry(x,y,timeLayout->geometry().width(),timeLayout->geometry().height());

    x = this->width() - sleepTime->geometry().width() - 26;
    y = this->height() - sleepTime->geometry().height() - 26;

    sleepTime->setGeometry(x,y,sleepTime->geometry().width(),sleepTime->geometry().height());

    if(centerWidget){
        centerWidget->adjustSize();
        centerWidget->setGeometry((width()-centerWidget->width())/2,(height()-centerWidget->height())/2,
                              centerWidget->width(),centerWidget->height());
    }

    ubuntuKylinlogo->setGeometry(26,26,ubuntuKylinlogo->width(),ubuntuKylinlogo->height());
}

void Screensaver::initUI()
{
    QFile qssFile(":/qss/assets/default.qss");
    if(qssFile.open(QIODevice::ReadOnly)) {
        setStyleSheet(qssFile.readAll());
    }
    qssFile.close();

    setDatelayout();
    setSleeptime();
  
    setCenterWidget();

    ubuntuKylinlogo = new QLabel(this);
    ubuntuKylinlogo->setPixmap(QPixmap(":/assets/logo.svg"));
    ubuntuKylinlogo->adjustSize();
}

void Screensaver::setDatelayout()
{
    timeLayout = new QWidget(this);
    QVBoxLayout *vtimeLayout = new QVBoxLayout(timeLayout);

    this->dateOfWeek = new QLabel(this);
    this->dateOfWeek->setText(QDate::currentDate().toString("dddd"));
    this->dateOfWeek->setObjectName("dateOfWeek");
    this->dateOfWeek->setAlignment(Qt::AlignCenter);
    vtimeLayout->addWidget(dateOfWeek);

    this->dateOfLocaltime = new QLabel(this);
    this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("hh:mm"));
    this->dateOfLocaltime->setObjectName("dateOfLocaltime");
    this->dateOfLocaltime->setAlignment(Qt::AlignCenter);
    vtimeLayout->addWidget(dateOfLocaltime);

    QWidget *dateWidget = new QWidget(this);
    this->dateOfDay = new QLabel(this);
    this->dateOfDay->setText(QDate::currentDate().toString("yy/MM/dd"));
    this->dateOfDay->setObjectName("dateOfDay");
    this->dateOfDay->setAlignment(Qt::AlignCenter);
    this->dateOfDay->adjustSize();

    QHBoxLayout *hdateLayout = new QHBoxLayout(dateWidget);
    hdateLayout->addWidget(dateOfDay);

    QString lang = qgetenv("LANG");
    if (!lang.isEmpty()){
        qDebug()<<"lang = "<<lang;
        if (lang.contains("zh_CN")){
            this->dateOfLunar = new QLabel(this);
            this->dateOfLunar->setText(date->getDateLunar());
            this->dateOfLunar->setObjectName("dateOfLunar");
            this->dateOfLunar->setAlignment(Qt::AlignCenter);
            this->dateOfLunar->adjustSize();
            hdateLayout->addWidget(dateOfLunar);
        }
    }
    dateWidget->adjustSize();

    vtimeLayout->addWidget(dateWidget);

    timeLayout->adjustSize();
}

void Screensaver::setSleeptime()
{
    sleepTime = new SleepTime(this);
    sleepTime->adjustSize();
     updateDate();
}

void Screensaver::updateDate()
{
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timer->start(1000);
}

void Screensaver::updateTime()
{
    this->dateOfWeek->setText(QDate::currentDate().toString("dddd"));
    this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("hh:mm"));
    this->dateOfDay->setText(QDate::currentDate().toString("yy/MM/dd"));
    if(sleepTime)
        sleepTime->setTime();

}

void Screensaver::setCenterWidget()
{
    QString lang = qgetenv("LANG");
    if (!lang.isEmpty()){
        qDebug()<<"lang = "<<lang;
        if (!lang.contains("zh_CN")){
            return;
        }
    }

    centerWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centerWidget);
    QLabel *label1 = new QLabel("因为有悔，所以披星戴月");
    label1->setObjectName("centerLabel");
    QLabel *label2 = new QLabel("因为有梦，所以奋不顾身");
    label2->setObjectName("centerLabel");
    layout->addWidget(label1);
    layout->addWidget(label2);
    adjustSize();

}
