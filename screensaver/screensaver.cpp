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
#include <QApplication>
#include <QTextCodec>
#include <QKeyEvent>
#include <QSplitterHandle>
#include <QDateTime>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QX11Info>
#include <QDBusInterface>
#include <QDBusReply>
#include <QApplication>
#include "screensaver.h"


Screensaver::Screensaver(QWidget *parent):
  QWidget(parent),
  date(new ChineseDate()),
  centerWidget(nullptr),
  dateOfLunar(nullptr),
  flag(0),
  background("")
{
    installEventFilter(this);
    initUI();
    m_background = new MBackground();

    QString backgroundFile = m_background->getCurrent();
    settings = new QGSettings("org.mate.background","",this);
    defaultBackground = settings->get("picture-filename").toString();

    if(backgroundFile.isEmpty())
       backgroundFile = defaultBackground;
    background = QPixmap(backgroundFile);

    QList<QLabel*> labelList = this->findChildren<QLabel *>();
    for(int i = 0;i<labelList.count();i++)
    {
        labelList.at(i)->setAlignment(Qt::AlignCenter);
    }
    setUpdateBackground();
    setUpdateCenterWidget();
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
    painter.drawPixmap(0,0,this->width(),this->height(),background);
    painter.setBrush(QColor(0,0,0,178));
    painter.drawRect(0,0,this->width(),this->height());

}

void Screensaver::resizeEvent(QResizeEvent */*event*/)
{
    float scale = 1.0;
    scale = (float)width()/1920;
    if(width() < 600 || height()<400){
        if(flag == 0)
        {
            QList<QLabel*> labelList = this->findChildren<QLabel *>();
            for(int i = 0;i<labelList.count();i++)
            {
                int fontsize = labelList.at(i)->font().pixelSize();
                const QString SheetStyle = QString("font-size:%1px;").arg(fontsize/2);
                labelList.at(i)->setStyleSheet(SheetStyle);

            }
            timeLayout->adjustSize();
            if(centerWidget)
                centerWidget->adjustSize();
        }
        flag = 1;
        buttonWidget->hide();
        if(sleepTime)
            sleepTime->hide();
    }

    int x = (this->width()-timeLayout->geometry().width())/2;
    int y = 129*scale;

    timeLayout->setGeometry(x,y,timeLayout->geometry().width(),timeLayout->geometry().height());

    if(sleepTime){
        x = this->width() - sleepTime->geometry().width() - 26*scale;
        y = this->height() - sleepTime->geometry().height() - 26*scale;
        sleepTime->setGeometry(x,y,sleepTime->geometry().width(),sleepTime->geometry().height());
    }
    if(centerWidget){
        centerWidget->adjustSize();
        centerWidget->setGeometry((width()-centerWidget->width())/2,(height()-centerWidget->height())/2,
                              centerWidget->width(),centerWidget->height());

        if((height()-centerWidget->height())/2 < timeLayout->y() + timeLayout->height())
            centerWidget->setGeometry((width()-centerWidget->width())/2,timeLayout->y() + timeLayout->height(),
                                  centerWidget->width(),centerWidget->height());
    }


    ubuntuKylinlogo->setGeometry(40*scale,40*scale,127*scale,42*scale);

    if(buttonWidget)
        buttonWidget->setGeometry(width() - 40*scale - buttonWidget->width(),31,buttonWidget->width(),buttonWidget->height());


    if(WallpaperButton)
        WallpaperButton->setGeometry(width() - 152*scale - WallpaperButton->width(),96*scale,WallpaperButton->width(),WallpaperButton->height());
}

void Screensaver::setUpdateCenterWidget()
{
    qsettings = new QSettings("/usr/share/ukui-screensaver/screensaver.ini",QSettings::IniFormat);
    qsettings->setIniCodec(QTextCodec::codecForName("UTF-8"));
    if(qsettings->childGroups().count()<=0)
        return;

    if(!centerWidget)
        return;

    QTimer *m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateCenterWidget()));
    m_timer->start(20000);

}

void Screensaver::updateCenterWidget()
{
    if(!centerWidget || !centerWidget->isVisible())
        return ;

    QStringList qlist = qsettings->childGroups();
    qsrand((unsigned)time(0));
    int index = qrand() % qlist.count() + 1;

    qsettings->beginGroup(QString::number(index));
    if(qsettings->contains("OL")){
        centerlabel1->setText(qsettings->value("OL").toString());
        centerlabel2->hide();
        authorlabel->setText(qsettings->value("author").toString());
        centerWidget->adjustSize();
        centerWidget->move((width()-centerWidget->width())/2,(height() - centerWidget->height())/2);
    }
    else if(qsettings->contains("FL"))
    {
        centerlabel1->setText(qsettings->value("FL").toString());
        centerlabel2->setText(qsettings->value("SL").toString());
        centerlabel2->show();
        authorlabel->setText(qsettings->value("author").toString());
        centerWidget->adjustSize();
        centerWidget->move((width()-centerWidget->width())/2,(height() - centerWidget->height())/2);
    }
    qsettings->endGroup();

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
    ubuntuKylinlogo->setObjectName("ubuntuKylinlogo");
    ubuntuKylinlogo->setPixmap(QPixmap(":/assets/logo.svg"));
    ubuntuKylinlogo->adjustSize();
    ubuntuKylinlogo->setScaledContents(true);

    escButton = new QPushButton(this);
    escButton->setObjectName("escButton");
    escButton->setText(tr("exit(Esc)"));
    escButton->setFixedSize(152,48);
    //connect(escButton,SIGNAL(clicked()),qApp,SLOT(quit()));

    prevButton = new QPushButton(this);
    prevButton->setObjectName("prevButton");
    prevButton->setFixedSize(48,48);
    prevButton->setIcon(QIcon(":/assets/prev.svg"));
    connect(prevButton,&QPushButton::clicked,this,[&]{
        QString path = m_background->getPrev();
        if(!path.isEmpty())
            background = QPixmap(path);
        repaint();
    });
    
    nextButton = new QPushButton(this);
    nextButton->setObjectName("nextButton");
    nextButton->setFixedSize(48,48);
    nextButton->setIcon(QIcon(":/assets/next.svg"));
    connect(nextButton,&QPushButton::clicked,this,[&]{
        QString path = m_background->getNext();
        if(!path.isEmpty())
            background = QPixmap(path);
        repaint();
    });
    
    settingsButton = new QPushButton(this);
    settingsButton->setObjectName("settingsButton");
    settingsButton->setFixedSize(48,48);
    settingsButton->setIcon(QIcon(":/assets/settings.svg"));
    connect(settingsButton,&QPushButton::clicked,this,[&]{
        if(WallpaperButton->isVisible())
            WallpaperButton->hide();
        else
            WallpaperButton->show();
    });

    buttonWidget = new QWidget(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setSpacing(16);
    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addWidget(settingsButton);
    buttonLayout->addWidget(escButton);
    buttonWidget->adjustSize();

    WallpaperButton = new QPushButton(this);
    WallpaperButton->setObjectName("WallpaperButton");
    WallpaperButton->setFixedSize(160,36);
    WallpaperButton->setIcon(QIcon(":/assets/wallpaper.svg"));
    WallpaperButton->setText(tr("Set as desktop wallpaper"));
    WallpaperButton->hide();
    connect(WallpaperButton,SIGNAL(clicked()),this,SLOT(setDesktopBackground()));

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
    if(sleepTime){
        if(!sleepTime->setTime()){
            timer->stop();
        }
    }
}

void Screensaver::setUpdateBackground()
{
    QTimer *m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateBackground()));
    m_timer->start(60000);
}

void Screensaver::updateBackground()
{
    QString path = m_background->getNext();
    if(!path.isEmpty()){
        background = QPixmap(path);
        repaint();
    }
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
    centerlabel1 = new QLabel("因为有悔，所以披星戴月");
    centerlabel1->setObjectName("centerLabel");
    centerlabel2 = new QLabel("因为有梦，所以奋不顾身");
    centerlabel2->setObjectName("centerLabel");
    authorlabel = new QLabel("");
    authorlabel->setObjectName("authorLabel");

    layout->addWidget(centerlabel1);
    layout->addWidget(centerlabel2);
    layout->addWidget(authorlabel);
    adjustSize();
}

void Screensaver::setDesktopBackground()
{
    WallpaperButton->hide();
    if(m_background->getCurrent().isEmpty())
        hide();
    settings->set("picture-filename",QVariant(m_background->getCurrent()));

    QDBusInterface * interface = new QDBusInterface("org.freedesktop.Accounts",
                                     "/org/freedesktop/Accounts",
                                     "org.freedesktop.Accounts",
                                     QDBusConnection::systemBus());

    if (!interface->isValid()){
        return;
    }

    QDBusReply<QDBusObjectPath> reply =  interface->call("FindUserByName", getenv("USER"));
    QString userPath;
    if (reply.isValid()){
        userPath = reply.value().path();
    }
    else {
        return;
    }

    QDBusInterface * useriFace = new QDBusInterface("org.freedesktop.Accounts",
                                                    userPath,
                                                    "org.freedesktop.Accounts.User",
                                                    QDBusConnection::systemBus());

    if (!useriFace->isValid()){
        return;
    }

    QDBusMessage msg = useriFace->call("SetBackgroundFile", m_background->getCurrent());
    if (!msg.errorMessage().isEmpty())
        qDebug() << "update user background file error: " << msg.errorMessage();

}
