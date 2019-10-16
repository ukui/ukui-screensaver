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
#include "configuration.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QMimeDatabase>
#include <QGSettings>
#include <QMimeType>
#include <ctime>

#define GSETTINGS_SCHEMA_SCREENSAVER "org.ukui.screensaver"
#define KEY_MODE "mode"
#define KEY_THEMES "themes"
#define KEY_IDLE_ACTIVATION_ENABLED "idle-activation-enabled"
#define KEY_LOCK_ENABLED "lock-enabled"
#define KEY_IMAGE_TRANSITION_EFFECT "image-transition-effect"
#define KEY_IMAGE_SWITCH_INTERVAL "image-switch-interval"
#define KEY_BACKGROUND "background"
#define XSCREENSAVER_DIRNAME "/usr/lib/xscreensaver"

Configuration::Configuration(QObject *parent) : QObject(parent)
{
	/* QGSettings for screensaver */
    gsettings = new QGSettings(GSETTINGS_SCHEMA_SCREENSAVER, "", this);
    connect(gsettings, &QGSettings::changed,
            this, &Configuration::onConfigurationChanged);

	/* Initiailization */
    mode = gsettings->get(KEY_MODE).toString();
    themes = gsettings->get(KEY_THEMES).toStringList();

    idleActivationEnabled = gsettings->get(
                        KEY_IDLE_ACTIVATION_ENABLED).toBool();
    lockEnabled = gsettings->get(KEY_LOCK_ENABLED).toBool();
    imageSwitchInterval = gsettings->get(KEY_IMAGE_SWITCH_INTERVAL).toInt();
    imageTSEffect = gsettings->get(KEY_IMAGE_TRANSITION_EFFECT).toInt();
    background = gsettings->get(KEY_BACKGROUND).toString();

    qDebug() << mode << themes;
    qDebug() << imageSwitchInterval << imageTSEffect;
	
        int FileisExist = 0;
    if(!background.isEmpty())
    {
        QFileInfo file(background);
        if(file.exists()==false)
                FileisExist = 0;
        else
                FileisExist = 1;
    }

    //如果org.ukui.screensaver background中的背景图片为空，则设为桌面背景
    if(background.isEmpty()||0==FileisExist)
    {
        QString currentDesktop = qgetenv("XDG_CURRENT_DESKTOP");
        if(currentDesktop == "UKUI" || currentDesktop == "MATE")
        {
            bgGsettings = new QGSettings("org.mate.background");
            background = bgGsettings->get("picture-filename").toString();
        }
        else if(currentDesktop == "ubuntu:GNOME")
        {
            bgGsettings = new QGSettings("org.gnome.desktop.background");
            background = bgGsettings->get("picture-uri").toString();
            //去除前缀：file:///usr/share/background/xxx.png
            background.remove(0, 7);
        }
    }
    qDebug() << "background: " << background;

    if(themes.count() == 1 && themes[0] == "kyccss-personal-slideshow")
        mode ="image";
}

/* Update member value when GSettings changed */
void Configuration::onConfigurationChanged(QString key)
{
	qDebug() << "GSettings value changed, key = " << key;
	if (key == KEY_MODE)
        mode = gsettings->get(KEY_MODE).toString();
	else if (key == KEY_THEMES)
        themes = gsettings->get(KEY_THEMES).toStringList();
	else if (key == KEY_IDLE_ACTIVATION_ENABLED)
        idleActivationEnabled = gsettings->get(KEY_IDLE_ACTIVATION_ENABLED).toBool();
	else if (key == KEY_LOCK_ENABLED)
        lockEnabled = gsettings->get(KEY_LOCK_ENABLED).toBool();
    else if(key == KEY_IMAGE_TRANSITION_EFFECT)
        imageTSEffect = gsettings->get(KEY_IMAGE_TRANSITION_EFFECT).toInt();
    else if(key == KEY_IMAGE_SWITCH_INTERVAL)
        imageSwitchInterval = gsettings->get(KEY_IMAGE_SWITCH_INTERVAL).toInt();
}

/*
 * Getter
 */

/* Get the executable path of xscreensaver */
ScreenSaver *Configuration::getScreensaver()
{
    QStringList modeStr{"blank-only", "random", "single", "image","default"};

    ScreenSaver *saver = new ScreenSaver;
    int index = modeStr.indexOf(mode);
    saver->mode = SaverMode(index);
    saver->interval = imageSwitchInterval;
    saver->effect = TransitionEffect(imageTSEffect);

    switch(index){
    case SAVER_DEFAULT:
        saver->path = getBackground();
        break;
    case SAVER_BLANK_ONLY:
        break;
    case SAVER_RANDOM:
    {
        qsrand((unsigned)time(0));
        int index = qrand() % themes.count();
        while(QString::compare(themes[index], "kyccss-personal-slideshow")==0)
        {
            index = qrand() % themes.count();
        }
        saver->path = getXScreensaverPath(themes[index]);
        break;
    }
    case SAVER_SINGLE:
        saver->path = getXScreensaverPath(themes[0]);
        break;
    case SAVER_IMAGE:
        QString lang = qgetenv("LANG");
        if (!lang.isEmpty()){
            qDebug()<<"lang = "<<lang;
            if (lang.contains("zh_CN")){
                saver->path = QDir::homePath() + "/图片";
                break;
            }
        }
        saver->path = QDir::homePath() + "/" + QStandardPaths::displayName(QStandardPaths::PicturesLocation);
        break;
    }
    return saver;
}

QString Configuration::getXScreensaverPath(const QString &theme)
{
    /* screensavers-ukui-binaryring => binaryring */
    QStringList strs = theme.split("-");
    QString str = strs.at(strs.size() - 1);
    return QString("%1/%2").arg(XSCREENSAVER_DIRNAME, str);
}

bool Configuration::ispicture(QString filepath)
{
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(filepath);
        qDebug() << "mime: " << filepath << mime.name();
        return mime.name().startsWith("image/");
}

QString Configuration::getBackground()
{
	if(ispicture(background))
                return background;
        else
                return "/usr/share/backgrounds/warty-final-ubuntukylin.jpg";
}

bool Configuration::xscreensaverActivatedWhenIdle()
{
	return idleActivationEnabled;
}

bool Configuration::lockWhenXScreensaverActivated()
{
	return lockEnabled;
}
