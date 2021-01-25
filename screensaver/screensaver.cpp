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
#include <QDate>
#include <QApplication>
#include <QTextCodec>
#include <QDateTime>
#include <QKeyEvent>
#include <QSplitterHandle>
#include <QCursor>
#include <QDateTime>
#include <QLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QHBoxLayout>
#include <QX11Info>
#include <QDBusInterface>
#include <QDBusReply>
#include <QApplication>
#include "screensaver.h"
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "commonfunc.h"

#define TIME_TYPE_SCHEMA "org.ukui.control-center.panel.plugins"

Screensaver::Screensaver(QWidget *parent):
  QWidget(parent),
  date(new ChineseDate()),
  centerWidget(nullptr),
  dateOfLunar(nullptr),
  flag(0),
  hasChanged(false),
  background(""),
  autoSwitch(nullptr),
  vboxFrame(nullptr),
  m_timer(nullptr)
{
    installEventFilter(this);
    setUpdateCenterWidget();
    initUI();
    m_background = new MBackground();

    settings = new QGSettings("org.mate.background","",this);
    defaultBackground = settings->get("picture-filename").toString();

    QString backgroundFile = defaultBackground;
    backgroundFile = getDefaultBackground(backgroundFile);
    background = QPixmap(backgroundFile);

    QList<QLabel*> labelList = this->findChildren<QLabel *>();
    for(int i = 0;i<labelList.count();i++)
    {
        labelList.at(i)->setAlignment(Qt::AlignCenter);
    }

    setUpdateBackground();

}

Screensaver::~Screensaver()
{

}

QString Screensaver::getDefaultBackground(QString background)
{
    if(ispicture(background))
        return background;
    
    return "/usr/share/backgrounds/warty-final-ubuntukylin.jpg";
}

bool Screensaver::eventFilter(QObject *obj, QEvent *event)
{
    /*	
    if(event->type() == 6){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() ==Qt::Key_Q || keyEvent->key() == Qt::Key_Escape){
            qApp->quit(); //需要 #include <QApplication> 头文件
        }
    }
    */ 
    if(obj == this){
        if(event->type()==QEvent::MouseButtonPress){
            XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(),XK_Escape), True, 1);
            XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(),XK_Escape), False, 1);
            XFlush(QX11Info::display());
        }
    }
    return false;
}

void Screensaver::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0,0,this->width(),this->height(),background);
    painter.setBrush(QColor(0,0,0,178));
    /*这里是为了不显示笔的线条*/
    painter.drawRect(-1,-1,this->width()+1,this->height()+1);
}

void Screensaver::resizeEvent(QResizeEvent */*event*/)
{
    float scale = 1.0;
    scale = (float)width()/1920;
    if(width() < 600 || height()<400){//当显示在控制面板上时，字体缩小三倍。
        if(flag == 0)
        {
            QList<QLabel*> labelList = this->findChildren<QLabel *>();
            for(int i = 0;i<labelList.count();i++)
            {
                int fontsize = labelList.at(i)->font().pixelSize();
                const QString SheetStyle = QString("font-size:%1px;").arg(fontsize/3);
                labelList.at(i)->setStyleSheet(SheetStyle);
            }
            QList<QWidget*> childList = timeLayout->findChildren<QWidget *>();
            for (int i = 0; i < childList.count(); ++i) {
                childList.at(i)->adjustSize();
            }
            timeLayout->adjustSize();
            if(centerWidget)
                centerWidget->adjustSize();
        }
        flag = 1;
        if(sleepTime)
            sleepTime->hide();
        if(settingsButton)
            settingsButton->hide();
        scale = 0.1;
    }

    int x = (this->width()-timeLayout->geometry().width())/2;
    int y = 59*scale;

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

    if(!getSystemDistrib().contains("ubuntu",Qt::CaseInsensitive)){
         ubuntuKylinlogo->setGeometry(40*scale,40*scale,107*scale,41*scale);
    }else{
         ubuntuKylinlogo->setGeometry(40*scale,40*scale,127*scale,42*scale);
    }


    if(settingsButton);
         settingsButton->setGeometry(width() - 40*scale - settingsButton->width(),40*scale,settingsButton->width(),settingsButton->height());

    if(vboxFrame)
        vboxFrame->setGeometry(width() - vboxFrame->width() - 40*scale,
                                settingsButton->geometry().bottom() + 12*scale,
                                vboxFrame->width(),vboxFrame->height());
}

void Screensaver::setUpdateCenterWidget()
{
    QString lang = qgetenv("LANG");
    QString cwdPath="/usr/share/ukui-screensaver/";
    QString languageDirPath=cwdPath+"language/";
    QString defaultLanguageFilePath=languageDirPath+"screensaver-en_US.ini";
    if (!lang.isEmpty()){
        qDebug()<<"lang = "<<lang;
        if(lang.contains('.')){
            lang=lang.split('.')[0];
            qDebug()<<"langStr = "<<lang;
        }
        QString languageFilePath=languageDirPath+"screensaver-"+lang+".ini";
        qDebug()<<"langnguageFile = "<<languageFilePath;
        QFileInfo fileInfo(languageFilePath);
        if (fileInfo.isFile()){
        	qsettings = new QSettings(languageFilePath,QSettings::IniFormat);
        }
        else{
            qsettings = new QSettings(defaultLanguageFilePath,QSettings::IniFormat);
	    }
    }
    else{
        qsettings = new QSettings(defaultLanguageFilePath,QSettings::IniFormat);
    }

    qsettings->setIniCodec(QTextCodec::codecForName("UTF-8"));
}

void Screensaver::updateCenterWidget(int index)
{
    if(!centerWidget )
        return ;

    QStringList qlist = qsettings->childGroups();
    if(qlist.count()<1)
        return;

    if(index<=1){
        qsrand((unsigned)time(0));
        index = qrand() % qlist.count() + 1;
    }
    qsettings->beginGroup(QString::number(index));
    if(qsettings->contains("OL")){
        centerlabel1->setText(qsettings->value("OL").toString());
        centerlabel2->hide();
        authorlabel->setText(qsettings->value("author").toString());
    }
    else if(qsettings->contains("FL"))
    {
        centerlabel1->setText(qsettings->value("FL").toString());
        centerlabel2->setText(qsettings->value("SL").toString());
        centerlabel2->show();
        authorlabel->setText(qsettings->value("author").toString());
    }
    
    centerWidget->adjustSize();
    centerWidget->setGeometry((width()-centerWidget->width())/2,(height()-centerWidget->height())/2,
                          centerWidget->width(),centerWidget->height());

    if((height()-centerWidget->height())/2 < timeLayout->y() + timeLayout->height())
        centerWidget->setGeometry((width()-centerWidget->width())/2,timeLayout->y() + timeLayout->height(),
                              centerWidget->width(),centerWidget->height());

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

    //logo
    ubuntuKylinlogo = new QLabel(this);
    ubuntuKylinlogo->setObjectName("ubuntuKylinlogo");
    ubuntuKylinlogo->setPixmap(QPixmap(":/assets/logo.svg"));
    ubuntuKylinlogo->adjustSize();
    ubuntuKylinlogo->setScaledContents(true);

    if(!getSystemDistrib().contains("ubuntu",Qt::CaseInsensitive)){
        ubuntuKylinlogo->setPixmap(QPixmap(":/assets/logo-kylin.svg"));
    }else{
	ubuntuKylinlogo->setPixmap(QPixmap(":/assets/logo.svg"));
    }

    //设置按钮
    settingsButton = new QPushButton(this);
    settingsButton->setObjectName("settingsButton");
    settingsButton->setFixedSize(48,48);
    settingsButton->setIcon(QIcon(":/assets/settings.svg"));
    settingsButton->installEventFilter(this);
    connect(settingsButton,&QPushButton::clicked,this,[&]{
        vboxFrame->setVisible(!vboxFrame->isVisible());

    });


    //设为壁纸按钮
    WallpaperButton = new QPushButton(this);
    WallpaperButton->setObjectName("WallpaperButton");
    WallpaperButton->setFixedHeight(36);
    WallpaperButton->setMinimumWidth(160);
    WallpaperButton->setIcon(QIcon(":/assets/wallpaper.svg"));
    WallpaperButton->setText(tr("Set as desktop wallpaper"));
    connect(WallpaperButton,SIGNAL(clicked()),this,SLOT(setDesktopBackground()));

    //自动切换
    QFrame *autoSwitch = new QFrame(this);
    autoSwitch->setObjectName("autoSwitch");
    autoSwitch->setFixedHeight(36);
    autoSwitch->setMinimumWidth(160);
    autoSwitchLabel = new QLabel(this);
    autoSwitchLabel->setObjectName("autoSwitchLabel");
    autoSwitchLabel->setText(tr("Automatic switching"));

    checkSwitch = new checkButton(this);

    //判断是否自动切换壁纸
    defaultSettings = new QGSettings("org.ukui.screensaver-default","",this);
    isAutoSwitch = defaultSettings->get("automatic-switching-enabled").toBool();

    checkSwitch->setAttribute(Qt::WA_DeleteOnClose);
    checkSwitch->setChecked(isAutoSwitch);
    connect(checkSwitch, &checkButton::checkedChanged, [=](bool checked){
         defaultSettings->set("automatic-switching-enabled",QVariant(checked));
         isAutoSwitch = checked;
         setUpdateBackground();
    });


    QHBoxLayout *hlayout = new QHBoxLayout(autoSwitch);
    hlayout->addWidget(autoSwitchLabel);
    hlayout->addWidget(checkSwitch);

    vboxFrame = new QFrame(this);
    vboxFrame->setObjectName("vboxFrame");
    vboxFrame->installEventFilter(this);
    QVBoxLayout *vlayout = new QVBoxLayout(vboxFrame);

    //分隔线
    QPushButton *line =new QPushButton(this);
    line->setWindowOpacity(0.08);
    line->setFocusPolicy(Qt::NoFocus);
    line->setMaximumHeight(1);

    //设置窗口
    vlayout->setContentsMargins(4,4,4,4);
    vlayout->setSpacing(4);
    vlayout->addWidget(WallpaperButton);
    vlayout->addWidget(line);
    vlayout->addWidget(autoSwitch);
    vlayout->setAlignment(autoSwitch,Qt::AlignCenter);
    vboxFrame->adjustSize();
    vboxFrame->hide();
}

void Screensaver::setDatelayout()
{
    if(QGSettings::isSchemaInstalled(TIME_TYPE_SCHEMA)){
        QGSettings *time_type = new QGSettings(TIME_TYPE_SCHEMA);
        QStringList keys = time_type->keys();
        if (keys.contains("hoursystem")) {
                timeType = time_type->get("hoursystem").toInt();
        }
    }

    timeLayout = new QWidget(this);
    QVBoxLayout *vtimeLayout = new QVBoxLayout(timeLayout);

//    this->dateOfWeek = new QLabel(this);
//    this->dateOfWeek->setText(QDate::currentDate().toString("ddd"));
//    this->dateOfWeek->setObjectName("dateOfWeek");
//    this->dateOfWeek->setAlignment(Qt::AlignCenter);
//    vtimeLayout->addWidget(dateOfWeek);

    this->dateOfLocaltime = new QLabel(this);
    if(timeType == 12)
        this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("ap hh:mm"));
    else
        this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("hh:mm"));

    this->dateOfLocaltime->setObjectName("dateOfLocaltime");
    this->dateOfLocaltime->setAlignment(Qt::AlignCenter);
    this->dateOfLocaltime->adjustSize();
    vtimeLayout->addWidget(dateOfLocaltime);

//    QWidget *dateWidget = new QWidget(this);
    this->dateOfDay = new QLabel(this);
    this->dateOfDay->setText(QDate::currentDate().toString("yyyy/MM/dd ddd"));
    this->dateOfDay->setObjectName("dateOfDay");
    this->dateOfDay->setAlignment(Qt::AlignCenter);
    this->dateOfDay->adjustSize();
    
//    QHBoxLayout *hdateLayout = new QHBoxLayout(dateWidget);
//    hdateLayout->addWidget(dateOfDay);
//    hdateLayout->addWidget(dateOfWeek);

//    QString lang = qgetenv("LANG");
//    if (!lang.isEmpty()){
//        qDebug()<<"lang = "<<lang;
//        if (lang.contains("zh_CN")){
//            this->dateOfLunar = new QLabel(this);
//            this->dateOfLunar->setText(date->getDateLunar());
//            this->dateOfLunar->setObjectName("dateOfLunar");
//            this->dateOfLunar->setAlignment(Qt::AlignCenter);
//            this->dateOfLunar->adjustSize();
//            hdateLayout->addWidget(dateOfLunar);
//        }
//    }
//   dateWidget->adjustSize();

    vtimeLayout->addWidget(this->dateOfDay);
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
    //this->dateOfWeek->setText(QDate::currentDate().toString("dddd"));
    if(timeType == 12)
        this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("ap hh:mm"));
    else
        this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("hh:mm"));

    this->dateOfDay->setText(QDate::currentDate().toString("yyyy/MM/dd ddd"));
    if(sleepTime){
        if(!sleepTime->setTime(QDateTime::currentDateTime())){
            sleepTime->hide();
            delete sleepTime; 
	    sleepTime=NULL;
        }
    }
}

void Screensaver::setUpdateBackground()
{
    if(isAutoSwitch){
        if(!m_timer){
            m_timer = new QTimer(this);
            connect(m_timer, SIGNAL(timeout()), this, SLOT(updateBackground()));
        }

        m_timer->start(300000);
    }
    else{
        if(m_timer && m_timer->isActive())
            m_timer->stop();
    }
}

void Screensaver::updateBackground()
{
    QString path = m_background->getRand();
    if(!path.isEmpty() && ispicture(path)){
        background = QPixmap(path);
        repaint();
        hasChanged=true;
    }
    updateCenterWidget(-1);
}

void Screensaver::setCenterWidget()
{
    QStringList qlist = qsettings->childGroups();
    if(qlist.count()<1)
        return;

    QDate date = QDate::currentDate();
    int days = date.daysTo(QDate(2100,1,1));
    int index = days%qlist.count()+1;

    QString configPath = QDir::homePath() + "/.ukui-screensaver-default.conf";
    QSettings settings1(configPath, QSettings::IniFormat);
    if(settings1.value("FIRST").toString().isEmpty()){
        settings1.setValue("FIRST",QDate::currentDate().toString("yy/MM/dd"));
        index = 1;
    }
    if(settings1.value("FIRST").toString() == QDate::currentDate().toString("yy/MM/dd"))
        index = 1;

    qsettings->beginGroup(QString::number(index));
    if(qsettings->contains("OL")){
        centerlabel1 = new QLabel(qsettings->value("OL").toString());
        centerlabel2 = new QLabel("");
        centerlabel2->hide();
        authorlabel = new QLabel(qsettings->value("author").toString());
    }
    else if(qsettings->contains("FL"))
    {
        centerlabel1 = new QLabel(qsettings->value("FL").toString());
        centerlabel2 = new QLabel(qsettings->value("SL").toString());
        centerlabel2->show();
        authorlabel = new QLabel(qsettings->value("author").toString());
    }
    centerlabel1->setObjectName("centerLabel");
    centerlabel2->setObjectName("centerLabel");
    authorlabel->setObjectName("authorLabel");

    qsettings->endGroup();

    centerWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centerWidget);

    QPushButton *line =new QPushButton(this);
    line->setWindowOpacity(0.08);
    line->setFocusPolicy(Qt::NoFocus);
    line->setMaximumHeight(1);

    layout->addWidget(centerlabel1);
    layout->addWidget(centerlabel2);
    layout->addWidget(line);
    layout->addWidget(authorlabel);

    adjustSize();
    centerWidget->setVisible(true);

}

void Screensaver::setDesktopBackground()
{
    vboxFrame->hide();
    QString mBackground;
	
    if(!hasChanged){
    	mBackground=defaultBackground; 
    }else{ 
        if(m_background->getCurrent().isEmpty())
            return;
        mBackground=m_background->getCurrent();
    }

    settings->set("picture-filename",QVariant(mBackground));

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

    QDBusMessage msg = useriFace->call("SetBackgroundFile", mBackground);
    if (!msg.errorMessage().isEmpty())
        qDebug() << "update user background file error: " << msg.errorMessage();

}
