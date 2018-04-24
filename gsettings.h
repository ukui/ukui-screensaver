#ifndef GSETTINGS_H
#define GSETTINGS_H

/* https://forum.qt.io/topic/7399 */
#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QObject>
extern "C" {
	#include <gio/gio.h>
}

class QGSettings
{
public:
	QGSettings(QString schema);

public:
	QString getString(QString key);
	int getInt(QString key);
	bool getBool(QString key);
	QList<QString> getStringList(QString key);

	bool setString(QString key, QString value);
	bool setInt(QString key, int value);
	bool setBool(QString key, bool value);
	bool setStringList(QString key, QList<QString> value);

private:
	GSettings *gsettings;
};

#endif // GSETTINGS_H
