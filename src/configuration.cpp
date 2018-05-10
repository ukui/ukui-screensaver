#include "configuration.h"
#include <QDebug>

#define GSETTINGS_SCHEMA_SCREENSAVER "org.ukui.screensaver"
#define KEY_MODE "mode"
#define KEY_THEMES "themes"
#define KEY_IDLE_ACTIVATION_ENABLED "idle-activation-enabled"
#define KEY_LOCK_ENABLED "lock-enabled"
#define XSCREENSAVER_DIRNAME "/usr/lib/xscreensaver"

#define GSETTINGS_SCHEMA_BACKGROUND "org.mate.background"
#define KEY_PICTURE_FILENAME "picture-filename"

#define GSETTINGS_SCHEMA_SESSION "org.ukui.session"
#define KEY_IDLE_DELAY "idle-delay"

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

	/* QGSettings for session */
	qgsettingsSession = new QGSettings(GSETTINGS_SCHEMA_SESSION);
	connect(qgsettingsSession, &QGSettings::valueChanged,
				this, &Configuration::onConfigurationChanged);


	/* Initiailization */
	mode = qgsettingsScreensaver->getString(KEY_MODE);
	themes = qgsettingsScreensaver->getStringList(KEY_THEMES);
	background = qgsettingsBackground->getString(KEY_PICTURE_FILENAME);
	idleDelay = qgsettingsSession->getInt(KEY_IDLE_DELAY);
	idleActivationEnabled = qgsettingsScreensaver->getBool(
						KEY_IDLE_ACTIVATION_ENABLED);
	lockEnabled = qgsettingsScreensaver->getBool(KEY_LOCK_ENABLED);
}

/* Update member value when GSettings changed */
void Configuration::onConfigurationChanged(QString key)
{
	qDebug() << "GSettings value changed, key = " << key;
	if (key == KEY_MODE)
		mode = qgsettingsScreensaver->getString(KEY_MODE);
	else if (key == KEY_THEMES)
		themes = qgsettingsScreensaver->getStringList(KEY_THEMES);
	else if (key == KEY_PICTURE_FILENAME)
		background = qgsettingsBackground->getString(KEY_PICTURE_FILENAME);
	else if (key == KEY_IDLE_DELAY)
		idleDelay = qgsettingsSession->getInt(KEY_IDLE_DELAY);
	else if (key == KEY_IDLE_ACTIVATION_ENABLED)
		idleActivationEnabled = qgsettingsScreensaver->getBool(
						KEY_IDLE_ACTIVATION_ENABLED);
	else if (key == KEY_LOCK_ENABLED)
		lockEnabled = qgsettingsScreensaver->getBool(
							KEY_LOCK_ENABLED);
}

/*
 * Getter
 */

/* Get the executable path of xscreensaver */
QString Configuration::getXScreensaver()
{
	QString selectedTheme;
	if (mode == "single") {
		selectedTheme = themes[0];
	} else if (mode == "random"){
		int randomIndex = qrand() % (themes.count());
		selectedTheme = themes[randomIndex];
	} else if (mode == "blank-only") { /* Note: blank not black */
		return QString("blank-only");
	} else {
		qDebug() << "Fatal error: unrecognized screensaver mode";
		return QString("blank-only");
	}
	/* screensavers-ukui-binaryring => binaryring */
	selectedTheme = selectedTheme.split("-")[2];
	return QString("%1/%2").arg(XSCREENSAVER_DIRNAME, selectedTheme);
}

QString Configuration::getBackground()
{
	return background;
}

int Configuration::getIdleDelay()
{
	return idleDelay;
}

bool Configuration::xscreensaverActivatedWhenIdle()
{
	return idleActivationEnabled;
}

bool Configuration::lockWhenXScreensaverActivated()
{
	return lockEnabled;
}
