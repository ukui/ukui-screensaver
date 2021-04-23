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
#include <QProcess>
#include <QDateTime>
#include <QKeyEvent>
#include <QSplitterHandle>
#include <QCursor>
#include <QColor>
#include <QPalette>
#include <QDateTime>
#include <QLayout>
#include <QStringList>
#include <QVBoxLayout>
#include <QDir>
#include <QByteArray>
#include <QImageReader>
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
  configuration(SCConfiguration::instance()),
  autoSwitch(nullptr),
  vboxFrame(nullptr),
  sleepTime(nullptr),
  lastPath(""),
  currentPath(""),
  switchTimer(nullptr),
  process(nullptr),
  cycleTime(300),
  timer(nullptr),
  isAutoSwitch(false),
  isCustom(false),
  backgroundPath(""),
  isShowRestTime(true),
  myTextLabel(nullptr),
  myTextWidget(nullptr),
  m_timer(nullptr)
{
    installEventFilter(this);

    qsrand(time(NULL));

    isCustom        =  configuration->getIsCustom();
    if(isCustom){
        cycleTime       =  configuration->getCycleTime();
        isAutoSwitch    =  configuration->getAutoSwitch();
        backgroundPath  =  configuration->getBackgroundPath();
        isShowRestTime  =  configuration->getShowRestTime();
        textIsCenter    =  configuration->getTextIsCenter();
        myText          =  configuration->getMyText();
    }

    setUpdateCenterWidget();
    initUI();
    m_background = new MBackground();

    QString backgroundFile = configuration->getDefaultBackground(); 
    background = QPixmap(backgroundFile);

    QList<QLabel*> labelList = this->findChildren<QLabel *>();
    for(int i = 0;i<labelList.count();i++)
    {
        labelList.at(i)->setAlignment(Qt::AlignCenter);
    }

    setUpdateBackground();
    connectSingles();
}

Screensaver::~Screensaver()
{

}

void Screensaver::connectSingles()
{
    connect(configuration, &SCConfiguration::autoSwitchChanged,
            this, &Screensaver::autoSwitchChanged);
    connect(configuration, &SCConfiguration::backgroundPathChanged,
            this, &Screensaver::backgroundPathChanged);
    connect(configuration, &SCConfiguration::cycleTimeChanged,
            this, &Screensaver::cycleTimeChanged);
    connect(configuration, &SCConfiguration::myTextChanged,
            this, &Screensaver::myTextChanged);
    connect(configuration, &SCConfiguration::showRestTimeChanged,
            this, &Screensaver::showRestTimeChanged);
    connect(configuration, &SCConfiguration::textIsCenterChanged,
            this, &Screensaver::textIsCenterChanged);
}

void Screensaver::autoSwitchChanged(bool isSwitch)
{
    isAutoSwitch = isSwitch;
    if(!isCustom)
        return ;
    setUpdateBackground();
}

void Screensaver::backgroundPathChanged(QString path)
{
    backgroundPath  = path;
    if(!isCustom)
        return ;
    stopSwitchImages();
    startSwitchImages();
}

void Screensaver::cycleTimeChanged(int cTime)
{
    cycleTime = cTime;
    if(!isCustom)
        return ;
    stopSwitchImages();
    startSwitchImages();
}

void Screensaver::myTextChanged(QString text)
{
    if(!isCustom)
        return ;

    myText = text;

    if(textIsCenter && centerWidget){
        if(centerlabel1)
            centerlabel1->setText(myText);
        if(centerlabel2){
            centerlabel2->setText("");
            centerlabel2->hide();
        }
        if(authorlabel){
            authorlabel->setText("");
            authorlabel->hide();
        }
        centerWidget->adjustSize();

        centerWidget->setGeometry((width()-centerWidget->width())/2,(height()-centerWidget->height())/2,
                                  centerWidget->width(),centerWidget->height());

        if((height()-centerWidget->height())/2 < timeLayout->y() + timeLayout->height())
            centerWidget->setGeometry((width()-centerWidget->width())/2,timeLayout->y() + timeLayout->height(),
                                      centerWidget->width(),centerWidget->height());
    }else{
        setRandomText();
        setRandomPos();
        if(centerWidget)
            centerWidget->hide();
    }
}

void Screensaver::showRestTimeChanged(bool isShow)
{
    isShowRestTime = isShow;
    if(!isCustom)
        return;

    setSleeptime(isShowRestTime);
}

void Screensaver::textIsCenterChanged(bool isCenter)
{
    textIsCenter = isCenter;
    if(!isCustom)
        return ;

    if(isCenter){
        if(myTextWidget)
            myTextWidget->hide();
        if(!centerWidget){
            setCenterWidget();
            resize(width(),height());
        }
        else
            centerWidget->show();
    }else{
        if(centerWidget)
            centerWidget->hide();
        setRandomText();
        setRandomPos();
    }
}

bool Screensaver::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == this){
        if(event->type()==QEvent::MouseButtonPress){
            XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(),XK_Escape), True, 1);
            XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(),XK_Escape), False, 1);
            XFlush(QX11Info::display());
	    /*
	    if(width() >200 && width() <500)
	    if(!process){
	    	process = new QProcess(this);
	    }
	    process->start("ukui-screensaver-command -s");
	    */
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
        if(myTextWidget){
            myTextLabel->setStyleSheet("font-size:5px;border-radius: 2px;background: rgba(255, 255, 255, 82%);color: #000000;padding: 4px 8px 4px 8px;");
            cycleLabel->setSize(QSize(5,5));
        }
        if(sleepTime)
            sleepTime->setSmallMode();
        if(settingsButton)
            settingsButton->hide();
        scale = 0.1;
    }

    int x = (this->width()-timeLayout->geometry().width())/2;
    int y = 59*scale;

    timeLayout->setGeometry(x,y,timeLayout->geometry().width(),timeLayout->geometry().height());

    if(sleepTime){
        x =  26*scale;
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

//    if(!getSystemDistrib().contains("ubuntu",Qt::CaseInsensitive)){
//         ubuntuKylinlogo->setGeometry(40*scale,40*scale,107*scale,41*scale);
//    }else{
//         ubuntuKylinlogo->setGeometry(40*scale,40*scale,127*scale,42*scale);
//    }


    if(settingsButton);
         settingsButton->setGeometry(width() - 40*scale - settingsButton->width(),40*scale,settingsButton->width(),settingsButton->height());

    if(vboxFrame)
        vboxFrame->setGeometry(width() - vboxFrame->width() - 40*scale,
                                settingsButton->geometry().bottom() + 12*scale,
                                vboxFrame->width(),vboxFrame->height());
    if(myTextWidget)
        setRandomPos();
}

void Screensaver::setRandomPos()
{
    myTextWidget->adjustSize();
    int x1 = 10;
    int x2 = width() - 10 - myTextWidget->width();
    int y1 = timeLayout->geometry().bottom() + 10;
    int y2;
    if(sleepTime)
    	y2 = sleepTime->geometry().top() - myTextWidget->height() - 10;
    else
        y2 = geometry().bottom() - myTextWidget->height() - 10;

    int x = 0;
    int y = 0;
    if(x2 > x1)
        x = qrand()%(x2 - x1) + x1;
    if(y2 > y1)
        y = qrand()%(y2 - y1) + y1;
	
    myTextWidget->move(x,y);

}

void Screensaver::setUpdateCenterWidget()
{
    QString lang = qgetenv("LANG");
    QString homePath=qgetenv("HOME");
    QString cwdPath="/usr/share/ukui-screensaver/";
    QString languageDirPath=cwdPath+"language/";
    QString defaultLanguageFilePath=languageDirPath+"screensaver-en_US.ini";
    qDebug()<<"homePath="<<homePath;
    if (!lang.isEmpty()){
        qDebug()<<"lang = "<<lang;
        if(lang.contains('.')){
            lang=lang.split('.')[0];
            qDebug()<<"langStr = "<<lang;
        }
        QString languageFilePath=languageDirPath+"screensaver-"+lang+".ini";
        QString homeLanguageFilePath=homePath+"/.config/ukui/screensaver-"+lang+".ini";
        qDebug()<<"langnguageFile = "<<languageFilePath;
        qDebug()<<"homeLanguageFilePath = "<<homeLanguageFilePath;
        QFileInfo fileInfo(languageFilePath);
        QFileInfo homeConfigFileInfo(homeLanguageFilePath);
        if (homeConfigFileInfo.isFile()){
            qsettings = new QSettings(homeLanguageFilePath,QSettings::IniFormat);
        }
        else if(fileInfo.isFile()){
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

void Screensaver::startSwitchImages()
{
    qDebug() << "ScreenSaver::startSwitchImages";
    QFileInfo fileInfo(backgroundPath);
    imagePaths.clear();
    if(fileInfo.isFile())
        return;
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    if(fileInfo.isDir()) {
        QDir dir(backgroundPath);
        QStringList files = dir.entryList(QDir::Files | QDir::Readable);
        for(QString file : files) {
            fileInfo.setFile(file);
            QString suffix = fileInfo.suffix();
            if(formats.contains(suffix.toUtf8()))
                imagePaths.push_back(backgroundPath + "/" + file);
        }
        if(!imagePaths.empty()) {
            switchTimer = new QTimer(this);
            connect(switchTimer, &QTimer::timeout, this, [&]{
                int index = qrand() % imagePaths.count();
                background  = QPixmap(imagePaths.at(index));
                repaint();
            });
            background  = QPixmap(imagePaths.at(0));
            switchTimer->start(cycleTime * 1000);
            repaint();
        }
    }
}

void Screensaver::stopSwitchImages()
{
    if(switchTimer && switchTimer->isActive())
        switchTimer->stop();
}

void Screensaver::onBackgroundChanged()
{
    opacity = 1.0;
    fadeTimer = new QTimer(this);
    connect(fadeTimer, &QTimer::timeout, this, [&]{
        opacity -= 0.1;
        if(opacity <= 0)
            fadeTimer->stop();
        else
            repaint();

    });
    fadeTimer->start(50);
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

    if(isCustom)
        setSleeptime(isShowRestTime);
    else
        setSleeptime(true);

    setCenterWidget();
    setRandomText();
    if(textIsCenter || myText == ""){
        myTextWidget->hide();
        centerWidget->show();
    }else{
        centerWidget->hide();
        myTextWidget->show();
    }

    //logo
//    ubuntuKylinlogo = new QLabel(this);
//    ubuntuKylinlogo->setObjectName("ubuntuKylinlogo");
//    ubuntuKylinlogo->setPixmap(QPixmap(":/assets/logo.svg"));
//    ubuntuKylinlogo->adjustSize();
//    ubuntuKylinlogo->setScaledContents(true);

//    if(!getSystemDistrib().contains("ubuntu",Qt::CaseInsensitive)){
//        ubuntuKylinlogo->setPixmap(QPixmap(":/assets/logo-kylin.svg"));
//    }else{
//        ubuntuKylinlogo->setPixmap(QPixmap(":/assets/logo.svg"));
//    }

    //设置按钮
    settingsButton = new QPushButton(this);
    settingsButton->setObjectName("settingsButton");
    settingsButton->setFixedSize(48,48);
    settingsButton->setIcon(QIcon(":/assets/settings.svg"));
    settingsButton->installEventFilter(this);
    connect(settingsButton,&QPushButton::clicked,this,[&]{
        vboxFrame->setVisible(!vboxFrame->isVisible());

    });
    settingsButton->hide();


    //设为壁纸按钮
    WallpaperButton = new QPushButton(this);
    WallpaperButton->setObjectName("WallpaperButton");
    WallpaperButton->setFixedHeight(36);
    WallpaperButton->setMinimumWidth(160);
    WallpaperButton->setIcon(QIcon(":/assets/wallpaper.svg"));
    WallpaperButton->setText(tr("Set as desktop wallpaper"));
    //connect(WallpaperButton,SIGNAL(clicked()),this,SLOTsetDesktopBackground()));

    //自动切换
    QFrame *autoSwitch = new QFrame(this);
    autoSwitch->setObjectName("autoSwitch");
    autoSwitch->setFixedHeight(36);
    autoSwitch->setMinimumWidth(160);
    autoSwitchLabel = new QLabel(this);
    autoSwitchLabel->setObjectName("autoSwitchLabel");
    autoSwitchLabel->setText(tr("Automatic switching"));

    checkSwitch = new checkButton(this);

    checkSwitch->setAttribute(Qt::WA_DeleteOnClose);
    checkSwitch->setChecked(isAutoSwitch);
/*    connect(checkSwitch, &checkButton::checkedChanged, [=](bool checked){
         defaultSettings->set("automatic-switching-enabled",QVariant(checked));
         isAutoSwitch = checked;
         setUpdateBackground();
    });
*/

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
    timeType = configuration->getTimeType();
    dateType = configuration->getDateType();

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
    if(dateType == "cn")
        this->dateOfDay->setText(QDate::currentDate().toString("yyyy/MM/dd ddd"));
    else
        this->dateOfDay->setText(QDate::currentDate().toString("yyyy-MM-dd ddd"));
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

void Screensaver::setSleeptime(bool Isshow)
{
    if(!sleepTime)
        sleepTime = new SleepTime(this);

    sleepTime->adjustSize();
    if(Isshow){
        updateDate();
        sleepTime->show();
    }
    else{
        sleepTime->hide();
        if(timer){
            timer->stop();
        }
    }
}

void Screensaver::updateDate()
{
    if(!timer){
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    }
    timer->start(1000);
    updateTime();
}

void Screensaver::updateTime()
{
    //this->dateOfWeek->setText(QDate::currentDate().toString("dddd"));
    if(timeType == 12)
        this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("ap hh:mm"));
    else
        this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("hh:mm"));

    if(dateType == "cn")
        this->dateOfDay->setText(QDate::currentDate().toString("yyyy/MM/dd ddd"));
    else
        this->dateOfDay->setText(QDate::currentDate().toString("yyyy-MM-dd ddd"));

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
        startSwitchImages();
    }
    else{
        stopSwitchImages();
        repaint();
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
   // updateCenterWidget(-1);
}

void Screensaver::setRandomText()
{
    if(!myTextWidget){
        myTextWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(myTextWidget);
        cycleLabel = new CycleLabel(this);
        layout->addWidget(cycleLabel);
        layout->setSpacing(16);
        myTextLabel = new QLabel(myTextWidget);
        myTextLabel->setObjectName("myText");
        myTextLabel->setBackgroundRole(QPalette::Base);
        myTextLabel->setAutoFillBackground(true);
        myTextLabel->setMaximumWidth(800);
        layout->addWidget(myTextLabel);
    }
	
    myTextLabel->setText(myText);
    myTextWidget->adjustSize();
    if(myText != "")
    	myTextWidget->setVisible(true);
    else
	myTextWidget->setVisible(false);
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
    if(isCustom){
        centerlabel1 = new QLabel(myText);
        centerlabel2 = new QLabel("");
        centerlabel2->hide();
        authorlabel = new QLabel("");
    }
    else if(qsettings->contains("OL")){
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
    layout->addWidget(centerlabel1);
    layout->addWidget(centerlabel2);

    if(!isCustom){
        QPushButton *line =new QPushButton(this);
        line->setWindowOpacity(0.08);
        line->setFocusPolicy(Qt::NoFocus);
        line->setMaximumHeight(1);
        layout->addWidget(line);
    }

    layout->addWidget(authorlabel);

    centerWidget->adjustSize();
    centerWidget->setGeometry((width()-centerWidget->width())/2,(height()-centerWidget->height())/2,
                              centerWidget->width(),centerWidget->height());
    centerWidget->setVisible(true);
}

/*
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

}*/
