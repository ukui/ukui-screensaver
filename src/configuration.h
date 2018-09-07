#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include "gsettings.h"

#include <QObject>
#include "screensaver.h"

class Configuration : public QObject
{
	Q_OBJECT
public:
	explicit Configuration(QObject *parent = nullptr);

public:
    ScreenSaver *getScreensaver();
	QString getBackground();
	bool xscreensaverActivatedWhenIdle();
	bool lockWhenXScreensaverActivated();

public Q_SLOTS:
	void onConfigurationChanged(QString key);

private:
    QString getXScreensaverPath(const QString &theme);

private:
	QGSettings *qgsettingsScreensaver;
	QGSettings *qgsettingsBackground;
    int mode;
	QList<QString> themes;
	QString background;
	bool idleActivationEnabled;
	bool lockEnabled;
    int imageTSEffect;
    int imageSwitchInterval;
};

#endif // CONFIGURATION_H
