#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <gsettings.h>

#include <QObject>

class Configuration : public QObject
{
	Q_OBJECT
public:
	explicit Configuration(QObject *parent = nullptr);

public:
	QString getXScreensaver();

public Q_SLOTS:
	void onConfigurationChanged(QString key);

private:
	QGSettings *qgsettings;
	QString mode;
	QList<QString> themes;
};

#endif // CONFIGURATION_H
