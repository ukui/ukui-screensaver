#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include "gsettings.h"

#include <QObject>

class Configuration : public QObject
{
	Q_OBJECT
public:
	explicit Configuration(QObject *parent = nullptr);

public:
	QString getXScreensaver();
	QString getBackground();
	bool xscreensaverActivatedWhenIdle();
	bool lockWhenXScreensaverActivated();

public Q_SLOTS:
	void onConfigurationChanged(QString key);

private:
	QGSettings *qgsettingsScreensaver;
	QGSettings *qgsettingsBackground;
	QString mode;
	QList<QString> themes;
	QString background;
	bool idleActivationEnabled;
	bool lockEnabled;
};

#endif // CONFIGURATION_H
