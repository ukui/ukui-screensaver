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
#include "scconfiguration.h"
#include "commonfunc.h"
#include <QDebug>
#include <QSettings>
#include <QGSettings>
#include <QString>

#define GSETTINGS_SCHEMA_SCREENSAVER 		"org.ukui.screensaver"
#define GSETTINGS_SCHEMA_MATE_BACKGROUND 	"org.mate.background"
#define GSETTINGS_SCHEMA_SCREENSAVER_DEFAULT 	"org.ukui.screensaver-default"
#define TIME_TYPE_SCHEMA 			"org.ukui.control-center.panel.plugins"

SCConfiguration* SCConfiguration::instance_ = nullptr;

SCConfiguration::SCConfiguration(QObject *parent) :
    QObject(parent),
    mgsettings(nullptr),
    ukgsettings(nullptr),
    udgsettings(nullptr),
    timegsettings(nullptr)
{
    initGsettings();
}

SCConfiguration* SCConfiguration::instance(QObject *parent)
{
    if(instance_ == nullptr)
        instance_ = new SCConfiguration(parent);
    return instance_;
}

void SCConfiguration::initGsettings()
{
    if(QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_MATE_BACKGROUND))
    	mgsettings = new QGSettings(GSETTINGS_SCHEMA_MATE_BACKGROUND, "", this);

    if(QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_SCREENSAVER))
    	ukgsettings = new QGSettings(GSETTINGS_SCHEMA_SCREENSAVER, "", this);
    
    if(QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_SCREENSAVER_DEFAULT))
    	udgsettings   = new QGSettings(GSETTINGS_SCHEMA_SCREENSAVER_DEFAULT,"",this);
    
    if(QGSettings::isSchemaInstalled(TIME_TYPE_SCHEMA))
    	timegsettings = new QGSettings(TIME_TYPE_SCHEMA,"",this);

    connect(udgsettings, &QGSettings::changed,
            this, &SCConfiguration::onConfigurationChanged);
    connect(ukgsettings, &QGSettings::changed,
            this, &SCConfiguration::onConfigurationChanged);
    connect(timegsettings, &QGSettings::changed,
            this, &SCConfiguration::onConfigurationChanged);
}

void SCConfiguration::initDefaultSettings()
{
     

}

void SCConfiguration::onConfigurationChanged(QString key)
{
    if(key == "cycleTime"){
        int cycleTime = getCycleTime();
        Q_EMIT cycleTimeChanged(cycleTime);
    }else if(key == "automaticSwitchingEnabled"){
        bool changed = getAutoSwitch();
        Q_EMIT autoSwitchChanged(changed);
    }else if(key == "backgroundPath"){
        QString path = getBackgroundPath();
        Q_EMIT backgroundPathChanged(path);
    }else if(key == "mytext"){
        QString text = getMyText();
        Q_EMIT myTextChanged(text);
    }else if(key == "showRestTime"){
        bool ret = getShowRestTime();
        Q_EMIT showRestTimeChanged(ret);
    }else if(key == "textIsCenter"){
        bool ret = getTextIsCenter();
        Q_EMIT textIsCenterChanged(ret);
    }else if(key == "showMessageEnabled"){
        bool ret = getMessageShowEnable();
        Q_EMIT messageShowEnableChanged(ret);
    }else if(key == "messageNumber"){
        int num = getMessageNumber();
        Q_EMIT messageNumberChanged(num);
    }else if(key == "hoursystem"){
        int timeType = timegsettings->get("hoursystem").toInt();
        Q_EMIT timeTypeChanged(timeType);
    }else if(key == "type"){
        QString dateType = timegsettings->get("date").toString();
        Q_EMIT dateTypeChanged(dateType);
    }
}

QString SCConfiguration::getDefaultBackground()
{
    QString backgroundFile = "";
    if(ukgsettings){
        backgroundFile = ukgsettings->get("background").toString();
    }

    if(ispicture(backgroundFile))
        return backgroundFile;
    else
        return "/usr/share/backgrounds/warty-final-ubuntukylin.jpg";
}

int SCConfiguration::getTimeType()
{
    int timeType = 24;
    if(timegsettings){
    	QStringList keys = timegsettings->keys();
	if (keys.contains("hoursystem")) {
            timeType = timegsettings->get("hoursystem").toInt();
        }
    }
    return timeType;
}

QString SCConfiguration::getDateType()
{
    QString dateType = "cn";
    if(timegsettings){
        QStringList keys = timegsettings->keys();
        if (keys.contains("date")) {
            dateType = timegsettings->get("date").toString();
        }
    }
    return dateType;
}

bool SCConfiguration::getAutoSwitch()
{
    bool ret = false;
    if(udgsettings){
        ret = udgsettings->get("automatic-switching-enabled").toBool();
    }

    return ret;
}

bool SCConfiguration::getIsCustom()
{
    bool ret = false;
    if(ukgsettings){
        ret = (ukgsettings->get("mode").toString() == "default-ukui-custom");
    }

    return ret;
}

bool SCConfiguration::getMessageShowEnable()
{
    bool ret = false;
    if(ukgsettings){
        ret = ukgsettings->get("show-message-enabled").toBool();
    }

    return ret;
}

int SCConfiguration::getMessageNumber()
{
    bool ret = false;
    if(ukgsettings){
        ret = (ukgsettings->get("mode").toString() == "default-ukui-custom");
    }

    return ret;
}

bool SCConfiguration::getShowRestTime()
{
    bool ret = true;
    if(udgsettings){
         QStringList keys = udgsettings->keys();
         ret = udgsettings->get("show-rest-time").toBool();
    }
    return ret;
}

int SCConfiguration::getCycleTime()
{
    int cycleTime = 300;
    if(udgsettings){
         cycleTime = udgsettings->get("cycle-time").toInt();
    }
    return cycleTime;
}

QString SCConfiguration::getBackgroundPath()
{
    QString backgroundPath = "/usr/share/backgrounds";
    if(udgsettings){
         backgroundPath = udgsettings->get("background-path").toString();
    }
    return backgroundPath;
}

bool SCConfiguration::getTextIsCenter()
{
    bool ret = true;
    if(udgsettings){
         ret = udgsettings->get("text-is-center").toBool();
    }
    return ret;
}

QString SCConfiguration::getMyText()
{
    QString myText = "";
    if(udgsettings){
         myText = udgsettings->get("mytext").toString();
    }
    return myText;
}
