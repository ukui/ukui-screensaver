#include "configuration.h"
#include <QDebug>

#define GSETTINGS_SCHEMA_SCREENSAVER "org.ukui.screensaver"
#define KEY_MODE "mode"
#define KEY_THEMES "themes"
#define XSCREENSAVER_DIRNAME "/usr/lib/xscreensaver"

Configuration::Configuration(QObject *parent) : QObject(parent)
{
	qgsettings = new QGSettings(GSETTINGS_SCHEMA_SCREENSAVER);
	connect(qgsettings, &QGSettings::valueChanged,
				this, &Configuration::onConfigurationChanged);

	/* Initiailization */
	mode = qgsettings->getString(KEY_MODE);
	themes = qgsettings->getStringList(KEY_THEMES);
}

/* Update member value when GSettings changed */
void Configuration::onConfigurationChanged(QString key)
{
	qDebug() << "GSettings value changed, key = " << key;
	if (key == KEY_MODE)
		mode = qgsettings->getString(KEY_MODE);
	else if (key == KEY_THEMES)
		themes = qgsettings->getStringList(KEY_THEMES);
}


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
