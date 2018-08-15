#include "configuration.h"
#include <QDebug>
#include <QFile>

#define GSETTINGS_SCHEMA_SCREENSAVER "org.ukui.screensaver"
#define KEY_MODE "mode"
#define KEY_THEMES "themes"
#define KEY_IDLE_ACTIVATION_ENABLED "idle-activation-enabled"
#define KEY_LOCK_ENABLED "lock-enabled"
#define KEY_IMAGE_TRANSITION_EFFECT "image-transition-effect"
#define KEY_IMAGE_SWITCH_INTERVAL "image-switch-interval"
#define KEY_BACKGROUND "background"
#define XSCREENSAVER_DIRNAME "/usr/lib/xscreensaver"

#define GSETTINGS_SCHEMA_BACKGROUND "org.mate.background"
#define KEY_PICTURE_FILENAME "picture-filename"

Configuration::Configuration(QObject *parent) : QObject(parent)
{
	/* QGSettings for screensaver */
    qgsettingsScreensaver = new QGSettings(GSETTINGS_SCHEMA_SCREENSAVER);
	connect(qgsettingsScreensaver, &QGSettings::valueChanged,
				this, &Configuration::onConfigurationChanged);

	/* QGSettings for background */
	qgsettingsBackground = new QGSettings(GSETTINGS_SCHEMA_BACKGROUND);
	connect(qgsettingsBackground, &QGSettings::valueChanged,
				this, &Configuration::onConfigurationChanged);

	/* Initiailization */
    mode = qgsettingsScreensaver->getEnum(KEY_MODE);
	themes = qgsettingsScreensaver->getStringList(KEY_THEMES);
	idleActivationEnabled = qgsettingsScreensaver->getBool(
						KEY_IDLE_ACTIVATION_ENABLED);
	lockEnabled = qgsettingsScreensaver->getBool(KEY_LOCK_ENABLED);
    imageSwitchInterval = qgsettingsScreensaver->getInt(KEY_IMAGE_SWITCH_INTERVAL);
    imageTSEffect = qgsettingsScreensaver->getEnum(KEY_IMAGE_TRANSITION_EFFECT);

    background = qgsettingsScreensaver->getString(KEY_BACKGROUND);
    QFile file(background);
    if(!file.exists())
        background = qgsettingsBackground->getString(KEY_PICTURE_FILENAME);

    qDebug() << imageSwitchInterval << imageTSEffect;
}

/* Update member value when GSettings changed */
void Configuration::onConfigurationChanged(QString key)
{
	qDebug() << "GSettings value changed, key = " << key;
	if (key == KEY_MODE)
        mode = qgsettingsScreensaver->getEnum(KEY_MODE);
	else if (key == KEY_THEMES)
		themes = qgsettingsScreensaver->getStringList(KEY_THEMES);
	else if (key == KEY_PICTURE_FILENAME)
		background = qgsettingsBackground->getString(KEY_PICTURE_FILENAME);
	else if (key == KEY_IDLE_ACTIVATION_ENABLED)
		idleActivationEnabled = qgsettingsScreensaver->getBool(
						KEY_IDLE_ACTIVATION_ENABLED);
	else if (key == KEY_LOCK_ENABLED)
        lockEnabled = qgsettingsScreensaver->getBool(KEY_LOCK_ENABLED);
    else if(key == KEY_IMAGE_TRANSITION_EFFECT)
        imageTSEffect = qgsettingsScreensaver->getEnum(KEY_IMAGE_TRANSITION_EFFECT);
    else if(key == KEY_IMAGE_SWITCH_INTERVAL)
        imageSwitchInterval = qgsettingsScreensaver->getInt(KEY_IMAGE_SWITCH_INTERVAL);
}

/*
 * Getter
 */

/* Get the executable path of xscreensaver */
ScreenSaver *Configuration::getScreensaver()
{
//	QString selectedTheme;
//	if (mode == "single") {
//		selectedTheme = themes[0];
//	} else if (mode == "random"){
//		int randomIndex = qrand() % (themes.count());
//		selectedTheme = themes[randomIndex];
//	} else if (mode == "blank-only") { /* Note: blank not black */
//		return QString("blank-only");
//	} else {
//		qDebug() << "Fatal error: unrecognized screensaver mode";
//		return QString("blank-only");
//	}
//	/* screensavers-ukui-binaryring => binaryring */
//    QStringList strs = selectedTheme.split("-");
//    selectedTheme = strs.at(strs.size() - 1);
//	return QString("%1/%2").arg(XSCREENSAVER_DIRNAME, selectedTheme);

    ScreenSaver *saver = new ScreenSaver;
    saver->mode = SaverMode(mode);

    switch(mode){
    case SAVER_BLANK_ONLY:
        break;
    case SAVER_RANDOM:
    {
        int index = qrand() % themes.count();
        saver->path = getXScreensaverPath(themes[index]);
        break;
    }
    case SAVER_SINGLE:
        saver->path = getXScreensaverPath(themes[0]);
        break;
    case SAVER_IMAGE:
        if(themes.size() <= 0)
            saver->path = background;
        else
            saver->path = themes[0];
        saver->interval = imageSwitchInterval;
        saver->effect = TransitionEffect(imageTSEffect);
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

QString Configuration::getBackground()
{
	return background;
}

bool Configuration::xscreensaverActivatedWhenIdle()
{
	return idleActivationEnabled;
}

bool Configuration::lockWhenXScreensaverActivated()
{
	return lockEnabled;
}
