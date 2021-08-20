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
#include "config.h"

#define TIME_TYPE_SCHEMA "org.ukui.control-center.panel.plugins"
#define THEME_TYPE_SCHENA "org.ukui.style"

#define GSETTINGS_SCHEMA_SCREENSAVER      "org.ukui.screensaver"
#define KEY_MESSAGE_NUMBER                "message-number"
#define KEY_MESSAGE_SHOW_ENABLED          "show-message-enabled"
#define KEY_HOURSYSTEM                    "hoursystem"
#define KEY_DATE_FORMAT                   "date"

QTime Screensaver::m_currentTime = QTime::currentTime();
extern bool bControlFlg;

Screensaver::Screensaver(QWidget *parent):
  QWidget(parent),
  switchTimer(nullptr),
  backgroundPath(""),
  cycleTime(300),
  isCustom(false),
  isShowRestTime(true),
  myTextLabel(nullptr),
  myPreviewLabel(nullptr),//预览label标签
  configuration(SCConfiguration::instance()),
  myTextWidget(nullptr),
  centerWidget(nullptr),
  sleepTime(nullptr),
  timer(nullptr),
  background(""),
  autoSwitch(nullptr),
  vboxFrame(nullptr),
  isAutoSwitch(false),
  flag(0),
  hasChanged(false),
  process(nullptr),
  screenLabel(nullptr),
  respondClick(false),
  m_weatherManager(new WeatherManager(this))
{
    installEventFilter(this);
  //  setWindowFlags(Qt::X11BypassWindowManagerHint);
    setUpdateCenterWidget();
    setMouseTracking(true);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    isCustom        =  configuration->getIsCustom();
    if(isCustom){
        cycleTime       =  configuration->getCycleTime();
        isAutoSwitch    =  configuration->getAutoSwitch();
        backgroundPath  =  configuration->getBackgroundPath();
        isShowRestTime  =  configuration->getShowRestTime();
        textIsCenter    =  configuration->getTextIsCenter();
        myText          =  configuration->getMyText();
    }

    initUI();
    m_background = new MBackground();

    QString backgroundFile = configuration->getDefaultBackground(); 
    background = QPixmap(backgroundFile);

    QList<QLabel*> labelList = this->findChildren<QLabel *>();
    for(int i = 0;i<labelList.count();i++)
    {
        labelList.at(i)->setAlignment(Qt::AlignCenter);
    }
#ifndef USE_INTEL
    updateBackgroundPath();
    startSwitchImages();

    connectSingles();
#endif
    QGSettings *themeSettings;
    if(QGSettings::isSchemaInstalled(TIME_TYPE_SCHEMA))
        themeSettings = new QGSettings(TIME_TYPE_SCHEMA,"",this);

    connect(themeSettings, &QGSettings::changed,
            this, &Screensaver::themeChanged);

    screenLabel = new QLabel(this);
    screenLabel->setObjectName("screenLabel");
    screenLabel->setText(tr("Picture does not exist"));
    screenLabel->adjustSize();
    screenLabel->hide();

}

Screensaver::~Screensaver()
{

}

void Screensaver::themeChanged()
{
    if(myTextLabel){
        QColor highLightColor = palette().color(QPalette::Base);
        QString stringColor = QString("rgba(%1,%2,%3,82%)")
                .arg(highLightColor.red())
                .arg(highLightColor.green())
                .arg(highLightColor.blue());
        QColor textColor = palette().color(QPalette::Text);
        QString textString = QString("rgb(%1,%2,%3)")
                .arg(textColor.red())
                .arg(textColor.green())
                .arg(textColor.blue());
        QColor borderColor = palette().color(QPalette::BrightText);
        QString borderString = QString("rgba(%1,%2,%3,85%)")
                .arg(borderColor.red())
                .arg(borderColor.green())
                .arg(borderColor.blue());

        if(width() < 600 || height()<400)
            myTextLabel->setStyleSheet(QString("font-size:5px;border-radius: 2px;background: %1;color: %2;padding: 4px 8px 4px 8px;border-width: 1px;border-style: solid;border-color:%3;") \
                                   .arg(stringColor).arg(textString).arg(borderString));
        else
            myTextLabel->setStyleSheet(QString("font-size:18px;border-radius: 6px;background: %1;color: %2;padding: 24px 48px 24px 48px;border-width: 1px;border-style: solid;border-color:%3;") \
                                       .arg(stringColor).arg(textString).arg(borderString));
    }
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
    connect(configuration, &SCConfiguration::messageNumberChanged,
            this, &Screensaver::onMessageNumberChanged);
    connect(configuration, &SCConfiguration::messageShowEnableChanged,
            this, &Screensaver::onMessageShowEnabledChanged);
    connect(configuration, &SCConfiguration::timeTypeChanged,
            this, &Screensaver::onTimeFormatChanged);
    connect(configuration, &SCConfiguration::dateTypeChanged,
            this, &Screensaver::onDateFormatChanged);
}

void Screensaver::onTimeFormatChanged(int type){
    timeType = type;
}

void Screensaver::onDateFormatChanged(QString type){
    dateType = type;
}

void Screensaver::onMessageNumberChanged(int num)
{
    int number = configuration->getMessageNumber();
    (configuration->getMessageShowEnable() && number > 0) ? showNotice() : hideNotice();
}

void Screensaver::onMessageShowEnabledChanged(bool enabled)
{
    int number = configuration->getMessageNumber();
    (configuration->getMessageShowEnable() && number > 0) ? showNotice() : hideNotice();
}

void Screensaver::autoSwitchChanged(bool isSwitch)
{
    if(!isCustom)
        return ;
    isAutoSwitch = isSwitch;
    if(!isSwitch){
        stopSwitchImages();
    }
    startSwitchImages();
}

/*
* 图片路径改变
*/
void Screensaver::backgroundPathChanged(QString path)
{
    backgroundPath  = path;
    if(!isCustom)
        return ;
    updateBackgroundPath();//更新图片路径
    stopSwitchImages();
    startSwitchImages();
}

void Screensaver::cycleTimeChanged(int cTime)
{
    cycleTime = cTime;
    if(!isCustom || !autoSwitch)
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
        else{
            centerWidget->show();
            myTextChanged(myText);
        }
    }else{
        if(centerWidget)
            centerWidget->hide();
        setRandomText();
        setRandomPos();
    }
}

bool Screensaver::eventFilter(QObject *obj, QEvent *event)
{
#ifndef USE_INTEL
    if(obj == this){
        if(event->type()==QEvent::MouseButtonPress){
            if(respondClick){
                if(!process){
                    process = new QProcess(this);
                }
                process->start("ukui-screensaver-command -s");
            }
        }
    }
#endif
    return false;
}

void Screensaver::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    /*时间未同步的时候重新刷新一次,主要是避免睡眠唤醒时的时间跳变*/
    if (m_currentTime.hour() != QTime::currentTime().hour() ||
            m_currentTime.minute() != QTime::currentTime().minute()) {
        updateTime();
    }

    if(isCustom && imagePaths.count()==0){
        painter.setBrush(QColor("#000000"));
        if(screenLabel->isHidden()){
            screenLabel->show();
        }
    }else{
        painter.drawPixmap(0,0,this->width(),this->height(), getPaddingPixmap());
        painter.setBrush(QColor(0,0,0,178));
        if(screenLabel->isVisible())
            screenLabel->hide();
    }
    /*这里是为了不显示笔的线条*/
    painter.drawRect(0,0,this->width(),this->height());
}

QPixmap Screensaver::getPaddingPixmap()
{
    if (background.width() == 0 || background.height() == 0)
    {
        return QPixmap();
    }

    bool useHeight;
    float scaled = 0.0;
    QPixmap scaledPixmap;
    QPixmap paddingPixmap;
    qint64 rw = qint64(this->height()) * qint64(background.width()) / qint64(background.height());

    useHeight = (rw >= this->width());
    if (useHeight) {
        scaled = float(this->height()) / float(background.height());
        scaledPixmap = background.scaled(background.width() * scaled, this->height());
        paddingPixmap = scaledPixmap.copy((background.width() * scaled - this->width()) / 2 , 0, this->width(), this->height());
    } else {
        scaled = float(this->width()) / float(background.width());
        scaledPixmap = background.scaled(this->width(), background.height() * scaled);
        paddingPixmap = scaledPixmap.copy(0 , (background.height() * scaled - this->height()) / 2,this->width(), this->height());
    }

    return paddingPixmap;
}

void Screensaver::addClickedEvent(){
    respondClick = true;
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
#ifdef USE_INTEL
                const QString SheetStyle = QString("font-size:%1px;").arg(fontsize/3);
#else
                const QString SheetStyle = QString("font-size:%1px;").arg(fontsize/4);
#endif
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
#ifndef USE_INTEL
        if(myTextWidget){

            QColor highLightColor = palette().color(QPalette::Base);
            QString stringColor = QString("rgba(%1,%2,%3,82%)")
                    .arg(highLightColor.red())
                    .arg(highLightColor.green())
                    .arg(highLightColor.blue());
            QColor textColor = palette().color(QPalette::Text);
            QString textString = QString("rgb(%1,%2,%3)")
                    .arg(textColor.red())
                    .arg(textColor.green())
                    .arg(textColor.blue());
            QColor borderColor = palette().color(QPalette::BrightText);
            QString borderString = QString("rgba(%1,%2,%3,85%)")
                    .arg(borderColor.red())
                    .arg(borderColor.green())
                    .arg(borderColor.blue());

            myTextLabel->setStyleSheet(QString("font-size:5px;border-radius: 2px;background: %1;color: %2;padding: 4px 8px 4px 8px;border-width: 1px;border-style: solid;border-color:%3;") \
                                   .arg(stringColor).arg(textString).arg(borderString));

            cycleLabel->setSize(QSize(5,5));
        }
        if(screenLabel)
            screenLabel->adjustSize();
        if(sleepTime)
            sleepTime->setSmallMode();
#endif
        scale = 0.1;
    }

#ifdef USE_INTEL
    int x = 840*scale;
    int y = 96*scale;

    m_weatherLaout->setGeometry((this->width()-m_weatherLaout->width())/2, y, m_weatherLaout->geometry().width(), m_weatherLaout->geometry().height());

    timeLayout->setGeometry((this->width()-timeLayout->width())/2,m_weatherLaout->geometry().bottom()+33,
                            timeLayout->geometry().width(),timeLayout->geometry().height());

    if(centerWidget){
        centerWidget->adjustSize();
        centerWidget->setGeometry((width()-centerWidget->width())/2,(height() * 85) / 100 - (centerWidget->height())/2,
                              centerWidget->width(),centerWidget->height());

        if((height()-centerWidget->height())/2 < timeLayout->y() + timeLayout->height())
            centerWidget->setGeometry((width()-centerWidget->width())/2,timeLayout->y() + timeLayout->height(),
                                  centerWidget->width(),centerWidget->height());
    }

    if (m_widgetNotice)
    {
        m_widgetNotice->setGeometry((this->width()-m_widgetNotice->width())/2, (centerWidget->y() + centerWidget->height() + 20),
                                    m_widgetNotice->geometry().width(), m_widgetNotice->geometry().height());
    }
#else
    int x = (this->width()-timeLayout->geometry().width())/2;
    int y = 59*scale;

    timeLayout->setGeometry(x,y,timeLayout->geometry().width(),timeLayout->geometry().height());

    if(sleepTime){
        x =  26*scale;
        y = this->height() - sleepTime->geometry().height() - 26*scale;
        sleepTime->setGeometry(x,y,sleepTime->geometry().width(),sleepTime->geometry().height());
    }

    if(screenLabel){
        screenLabel->setGeometry((width() - screenLabel->width())/2,y,screenLabel->width(),screenLabel->height());
    }

    if(centerWidget){
        centerWidget->adjustSize();
        centerWidget->setGeometry((width()-centerWidget->width())/2,(height()-centerWidget->height())/2,
                              centerWidget->width(),centerWidget->height());

        if((height()-centerWidget->height())/2 < timeLayout->y() + timeLayout->height())
            centerWidget->setGeometry((width()-centerWidget->width())/2,timeLayout->y() + timeLayout->height(),
                                  centerWidget->width(),centerWidget->height());
    }

    if(myTextWidget)
        setRandomPos();
#endif
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
    //QString lang = qgetenv("LANG");
    QString lang = QLocale::system().name();
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
        QString jdLanguageFilePath=languageDirPath+"screensaver-jd" + ".ini";
        qDebug()<<"langnguageFile = "<<languageFilePath;
        qDebug()<<"homeLanguageFilePath = "<<homeLanguageFilePath;
        QFileInfo fileInfo(languageFilePath);
        QFileInfo homeConfigFileInfo(homeLanguageFilePath);
        QFileInfo jdConfigFileInfo(jdLanguageFilePath);
        bool useJd = false;

        QDate date1(2021,6,20);
        QDate date2(2021,7,31);
        if(QDate::currentDate() >= date1 && QDate::currentDate()<=date2){
            useJd = true;
        }

        if(useJd && jdConfigFileInfo.isFile()){
            qsettings = new QSettings(jdLanguageFilePath,QSettings::IniFormat);
        }else if (homeConfigFileInfo.isFile()){
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

void Screensaver::updateBackgroundPath()
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
            if(formats.contains(suffix.toUtf8()) && file.right(4) != ".ico" && file.right(4) != ".tga"){
                // qDebug() << "file:" <<file;
                imagePaths.push_back(backgroundPath + "/" + file);
            }
        }
    }
}
void Screensaver::enterEvent(QEvent*){
    // qDebug() << "enter ScreenSaver::enterEvent";
    //当前是否是控制面板窗口
    if(bControlFlg){
        setPreviewText(true);
    }   
}

void Screensaver::leaveEvent(QEvent*){
    // qDebug() << "enter ScreenSaver::leaveEvent";
    setPreviewText(false);
    // setToolTip("离开");
}

void Screensaver::startSwitchImages()
{
    if(!imagePaths.empty()) {
        background  = QPixmap(imagePaths.at(0));

        if(!switchTimer){
            switchTimer = new QTimer(this);
            connect(switchTimer, &QTimer::timeout, this, [&]{
                int index = qrand() % imagePaths.count();
                background  = QPixmap(imagePaths.at(index));
                repaint();
            });
        }
        if(isAutoSwitch){
            switchTimer->start(cycleTime * 1000);
        }
        repaint();
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
        index = qrand() % qlist.count() + 1;
    }
    qsettings->beginGroup(QString::number(index));
    if(qsettings->contains("OL")){
        centerlabel1->setText(qsettings->value("OL").toString());
        centerlabel2->hide();
#ifndef USE_INTEL
        authorlabel->setText(qsettings->value("author").toString());
#endif
    }
    else if(qsettings->contains("FL"))
    {
        centerlabel1->setText(qsettings->value("FL").toString());
        centerlabel2->setText(qsettings->value("SL").toString());
        centerlabel2->show();
#ifndef USE_INTEL
        authorlabel->setText(qsettings->value("author").toString());
#endif
    }

#ifdef USE_INTEL
    if(qsettings->contains("author") && !qsettings->value("author").toString().isEmpty())
    {
        authorlabel->setText(qsettings->value("author").toString());
        authorlabel->show();
    }
    else
    {
        authorlabel->setText("");
        authorlabel->hide();
    }
#endif

    centerWidget->adjustSize();

#ifdef USE_INTEL
    centerWidget->setGeometry((width()-centerWidget->width())/2,(height() * 85) / 100 - (centerWidget->height())/2,
                          centerWidget->width(),centerWidget->height());

    if((height()-centerWidget->height())/2 < timeLayout->y() + timeLayout->height())
        centerWidget->setGeometry((width()-centerWidget->width())/2,timeLayout->y() + timeLayout->height(),
                              centerWidget->width(),centerWidget->height());

#else
    centerWidget->setGeometry((width()-centerWidget->width())/2,(height()-centerWidget->height())/2,
                              centerWidget->width(),centerWidget->height());

    if((height()-centerWidget->height())/2 < timeLayout->y() + timeLayout->height())
        centerWidget->setGeometry((width()-centerWidget->width())/2,timeLayout->y() + timeLayout->height(),
                                  centerWidget->width(),centerWidget->height());
#endif
    qsettings->endGroup();

}

void Screensaver::initUI()
{
    QFile qssFile;

#ifdef USE_INTEL
    qssFile.setFileName(":/qss/assets/default-intel.qss");
#else
    qssFile.setFileName(":/qss/assets/default.qss");
#endif

    if(qssFile.open(QIODevice::ReadOnly)) {
        setStyleSheet(qssFile.readAll());
    }
    qssFile.close();

#ifdef USE_INTEL
    setWeatherLayout();
    setDatelayout();
    setCenterWidget();
    setNoticeLaout();
    m_weatherManager->getWeather();
    connect(m_weatherManager, &WeatherManager::onWeatherUpdate,
            this, &Screensaver::getWeatherFinish);
#else
    if(isCustom)
        setSleeptime(isShowRestTime);
    else
        setSleeptime(true);
    setDatelayout();
    setCenterWidget();
    setRandomText();
    if(textIsCenter || myText == ""){
        myTextWidget->hide();
        centerWidget->show();
    }else{
        centerWidget->hide();
        myTextWidget->show();
    }
#endif
}

void Screensaver::setDatelayout()
{
    timeType = configuration->getTimeType();
    dateType = configuration->getDateType();
#ifdef USE_INTEL
    timeLayout = new QWidget(this);
    timeLayout->setFixedSize(((timeType == 12) ? 397:326), 117);

    QHBoxLayout *htimeLayout = new QHBoxLayout(timeLayout);
    htimeLayout->setContentsMargins(0,0,0,0);

    QWidget *timeWidget = new QWidget(this);
    timeWidget->setFixedSize(235, 117);
    QGridLayout *gtimeLayout = new QGridLayout(timeWidget);
    gtimeLayout->setContentsMargins(0,0,0,12);

    this->dateOfLocaltimeHour = new QLabel(this);
    this->dateOfLocaltimeHour->setObjectName("dateOfLocaltime");
    this->dateOfLocaltimeHour->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->dateOfLocaltimeHour->setFixedSize(107, 96);

    this->dateofLocaltimeColon = new QLabel(this);
    this->dateofLocaltimeColon->setObjectName("dateOfLocaltime");
    this->dateofLocaltimeColon->setAlignment(Qt::AlignCenter);
    this->dateofLocaltimeColon->setFixedSize(21, 96);
    this->dateofLocaltimeColon->setText(":");

    this->dateOfLocaltimeMinute = new QLabel(this);
    this->dateOfLocaltimeMinute->setObjectName("dateOfLocaltime");
    this->dateOfLocaltimeMinute->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->dateOfLocaltimeMinute->setFixedSize(107, 96);

    gtimeLayout->setSpacing(0);
    gtimeLayout->setRowMinimumHeight(0, 9);
    gtimeLayout->setRowMinimumHeight(1, 87);
    gtimeLayout->setRowMinimumHeight(2, 9);

    gtimeLayout->setColumnMinimumWidth(0, 107);
    gtimeLayout->setColumnMinimumWidth(1, 21);
    gtimeLayout->setColumnMinimumWidth(2, 107);

    gtimeLayout->addWidget(dateOfLocaltimeHour, 1, 0, 2, 1);
    gtimeLayout->addWidget(dateofLocaltimeColon, 0, 1, 2, 1);
    gtimeLayout->addWidget(dateOfLocaltimeMinute, 1, 2, 2, 1);

    QWidget *dateWidget = new QWidget(this);
    dateWidget->setFixedSize(((timeType == 12) ? 154:83), 117);
    QVBoxLayout *vDateLaout = new QVBoxLayout(dateWidget);
    vDateLaout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    vDateLaout->setContentsMargins(0,18,0,12);

    this->dateOfWeek = new QLabel(this);
    this->dateOfWeek->setObjectName("dateOfWeek");
    this->dateOfWeek->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->dateOfWeek->setFixedWidth(83);

    this->dateOfDay = new QLabel(this);
    this->dateOfDay->setObjectName("dateOfDay");
    this->dateOfDay->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->dateOfDay->setFixedSize(((timeType == 12) ? 154:83),32);
    updateTime();
    updateDate();

    this->dateOfWeek->adjustSize();
    vDateLaout->addWidget(dateOfWeek);
    vDateLaout->setSpacing(4);
    vDateLaout->addWidget(dateOfDay);

    htimeLayout->addWidget(timeWidget);
    htimeLayout->addSpacing(8);
    htimeLayout->addWidget(dateWidget);
 #else
    timeLayout = new QWidget(this);
    QVBoxLayout *vtimeLayout = new QVBoxLayout(timeLayout);

    this->dateOfLocaltime = new QLabel(this);
    if(timeType == 12)
        this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("A hh:mm"));
    else
        this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("hh:mm"));

    this->dateOfLocaltime->setObjectName("dateOfLocaltime");
    this->dateOfLocaltime->setAlignment(Qt::AlignCenter);
    this->dateOfLocaltime->adjustSize();
    vtimeLayout->addWidget(dateOfLocaltime);

    this->dateOfDay = new QLabel(this);
    if(dateType == "cn")
        this->dateOfDay->setText(QDate::currentDate().toString("yyyy/MM/dd ddd"));
    else
        this->dateOfDay->setText(QDate::currentDate().toString("yyyy-MM-dd ddd"));
    this->dateOfDay->setObjectName("dateOfDay");
    this->dateOfDay->setAlignment(Qt::AlignCenter);
    this->dateOfDay->adjustSize();

    vtimeLayout->addWidget(this->dateOfDay);
    timeLayout->adjustSize();
    updateDate();
#endif

}

void Screensaver::setWeatherLayout()
{
    m_weatherLaout = new QWidget(this);
    QHBoxLayout *hWeatherLayout = new QHBoxLayout(m_weatherLaout);
    hWeatherLayout->setContentsMargins(0, 0, 0, 0);

    this->m_weatherIcon = new QLabel(this);
    this->m_weatherArea = new QLabel(this);
    this->m_weatherCond = new QLabel(this);
    this->m_weatherTemperature = new QLabel(this);

    m_weatherIcon->setPixmap(m_weatherManager->getWeatherIcon());
    m_weatherArea->setText(m_weatherManager->getCityName());
    if (!m_weatherManager->getCond().isEmpty())
        m_weatherCond->setText("·" + m_weatherManager->getCond());
    m_weatherTemperature->setText(m_weatherManager->getTemperature());

    m_weatherArea->setStyleSheet("font-size:26px;color:#ffffff");
    m_weatherCond->setStyleSheet("font-size:26px;color:#ffffff");
    m_weatherTemperature->setStyleSheet("font-size:26px;color:#ffffff");

    hWeatherLayout->addWidget(m_weatherIcon);
    hWeatherLayout->addSpacing(8);
    hWeatherLayout->addWidget(m_weatherArea);
    hWeatherLayout->addWidget(m_weatherCond);
    hWeatherLayout->addSpacing(8);
    hWeatherLayout->addWidget(m_weatherTemperature);
    m_weatherLaout->adjustSize();
}

void Screensaver::setSleeptime(bool Isshow)
{
    if(!sleepTime)
        sleepTime = new SleepTime(this);

    sleepTime->adjustSize();
    if(Isshow){
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
#ifdef USE_INTEL
    QLocale locale(QLocale::system().name());

    QTimeZone timeZone(QString::fromLatin1(QTimeZone::systemTimeZoneId()).toLatin1());
    QDateTime tzNow = QDateTime::currentDateTime().toTimeZone(timeZone);

    QString time;

    if (timeType == 12)
    {
        time = tzNow.toString("hh:mm AP").split(" ").at(0);
        this->dateOfWeek->setText(tzNow.toString("AP"));
        if(dateType == "en")
        {
            this->dateOfDay->setText(tzNow.toString("ddd MM-dd"));
        } else {
            this->dateOfDay->setText(tzNow.toString("ddd MM/dd"));
        }
    } else {
        time = tzNow.toString("hh:mm");
        this->dateOfWeek->setText(tzNow.toString("ddd"));
        if(dateType == "en")
        {
            this->dateOfDay->setText(tzNow.toString("MM-dd"));
        } else {
            this->dateOfDay->setText(tzNow.toString("MM/dd"));
        }
    }

    this->dateOfLocaltimeHour->setText(time.split(":").at(0));
    this->dateOfLocaltimeMinute->setText(time.split(":").at(1));

    m_currentTime = QTime::currentTime();
#else
    if(timeType == 12)
        this->dateOfLocaltime->setText(QDateTime::currentDateTime().toString("A hh:mm"));
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
#endif
}

void Screensaver::setUpdateBackground()
{

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
        QColor highLightColor = palette().color(QPalette::Base);
        QString stringColor = QString("rgba(%1,%2,%3,82%)")
                .arg(highLightColor.red())
                .arg(highLightColor.green())
                .arg(highLightColor.blue());
        QColor textColor = palette().color(QPalette::Text);
        QString textString = QString("rgb(%1,%2,%3)")
                .arg(textColor.red())
                .arg(textColor.green())
                .arg(textColor.blue());
        QColor borderColor = palette().color(QPalette::BrightText);
        QString borderString = QString("rgba(%1,%2,%3,85%)")
                .arg(borderColor.red())
                .arg(borderColor.green())
                .arg(borderColor.blue());
        myTextLabel->setStyleSheet(QString("font-size:18px;border-radius: 6px;background: %1;color: %2;padding: 24px 48px 24px 48px;border-width: 1px;border-style: solid;border-color:%3;") \
                                   .arg(stringColor).arg(textString).arg(borderString));
        layout->addWidget(myTextLabel);
    }
	
    myTextLabel->setText(myText);
    myTextWidget->adjustSize();
    if(myText != "")
    	myTextWidget->setVisible(true);
    else
        myTextWidget->setVisible(false);
}

void Screensaver::setPreviewText(bool bVisible)
{
    if(!myPreviewLabel){
        myPreviewLabel = new QLabel(this);
        myPreviewLabel->setFixedSize(58,30);

        //设置样式
        myPreviewLabel->setStyleSheet("background-color: rgb(38,38,38); border-radius: 0px; color:white;");

        //先采取固定大小方式
        myPreviewLabel->move(120,142);
        myPreviewLabel->setAlignment(Qt::AlignCenter);
    }
	
    myPreviewLabel->setText(tr("View"));
    myPreviewLabel->adjustSize();
    
    myPreviewLabel->setVisible(bVisible);  

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
#ifndef USE_INTEL
        authorlabel = new QLabel(qsettings->value("author").toString());
#endif
    }
    else if(qsettings->contains("FL"))
    {
        centerlabel1 = new QLabel(qsettings->value("FL").toString());
        centerlabel2 = new QLabel(qsettings->value("SL").toString());
        centerlabel2->show();
#ifndef USE_INTEL
        authorlabel = new QLabel(qsettings->value("author").toString());
#endif
    }

#ifdef USE_INTEL
    if(qsettings->contains("author")  && !qsettings->value("author").toString().isEmpty())
    {
        authorlabel = new QLabel(qsettings->value("author").toString());
        authorlabel->show();
    }
    else
    {
        authorlabel = new QLabel("");
        authorlabel->hide();
    }
#endif

    centerlabel1->setObjectName("centerLabel");
    centerlabel2->setObjectName("centerLabel");
    authorlabel->setObjectName("authorLabel");

    qsettings->endGroup();

#ifdef USE_INTEL
    //设置背景透明，qss中更改为透明不生效
    centerlabel1->setStyleSheet("QLabel{background-color: transparent;}");
    centerlabel2->setStyleSheet("QLabel{background-color: transparent;}");
    authorlabel->setStyleSheet("QLabel{background-color: transparent;}");

    centerWidget = new QWidget(this);
    centerWidget->setStyleSheet("QWidget{background:rgb(0,0,0,64);border-radius:16px}");
    QVBoxLayout *layout = new QVBoxLayout(centerWidget);

//    QPushButton *line =new QPushButton(this);
//    line->setWindowOpacity(0.08);
//    line->setFocusPolicy(Qt::NoFocus);
//    line->setMaximumHeight(1);

    layout->addWidget(centerlabel1);
    layout->addWidget(centerlabel2);
//    layout->addWidget(line);
    layout->addWidget(authorlabel);


    adjustSize();
    centerWidget->setVisible(true);
#else
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
#endif
}

void Screensaver::getWeatherFinish(QString city, QString cond, QString tmp)
{
    qDebug() << "getWeatherFinish";
    qDebug() << city << "," << cond << "," << tmp;

    this->m_weatherIcon->setPixmap(m_weatherManager->getWeatherIcon(cond));
    this->m_weatherArea->setText(city);

    if(!cond.isEmpty())
    {
        this->m_weatherCond->show();
        this->m_weatherCond->setText("·" + cond);
    }
    else
        this->m_weatherCond->hide();

    if(!tmp.isEmpty())
    {
        this->m_weatherTemperature->show();
        this->m_weatherTemperature->setText(tmp);
    }
    else
        this->m_weatherTemperature->hide();

    m_weatherLaout->adjustSize();
    m_weatherLaout->setGeometry((this->width()-m_weatherLaout->width())/2,96 * (float)width()/1920,
                                m_weatherLaout->geometry().width(), m_weatherLaout->geometry().height());
}

void Screensaver::setNoticeLaout()
{
    m_widgetNotice = new QWidget(this);
    QHBoxLayout *hNoticeLayout = new QHBoxLayout(m_widgetNotice);
    hNoticeLayout->setContentsMargins(0, 0, 0, 0);
    m_labelNoticeIcon = new QLabel(this);
    m_labelNoticeMessage = new QLabel(this);
    m_labelNoticeMessage->setStyleSheet("font-size:16px;color:#ffffff");

    m_labelNoticeIcon->setPixmap(QPixmap(":/assets/message.png"));

    m_labelNoticeMessage->setText(tr("You have new notification"));

    int num = configuration->getMessageNumber();
    (configuration->getMessageShowEnable() && num > 0)? showNotice() : hideNotice();

    hNoticeLayout->addWidget(m_labelNoticeIcon);
    hNoticeLayout->addWidget(m_labelNoticeMessage);
    m_widgetNotice->adjustSize();
}

//显示新消息通知
void Screensaver::showNotice()
{
    m_widgetNotice->show();
}

void Screensaver::hideNotice()
{
    m_widgetNotice->hide();
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
