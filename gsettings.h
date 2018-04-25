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

class QGSettings : public QObject
{
	Q_OBJECT

public:
	explicit QGSettings(QString schema, QObject *parent = nullptr);

public:
	QString getString(QString key);
	int getInt(QString key);
	bool getBool(QString key);
	QList<QString> getStringList(QString key);

	bool setString(QString key, QString value);
	bool setInt(QString key, int value);
	bool setBool(QString key, bool value);
	bool setStringList(QString key, QList<QString> value);
	static void changedCallback(GSettings *gsettings, const gchar *key,
							gpointer user_data);

Q_SIGNALS:
	void valueChanged(QString key);

private:
	GSettings *gsettings;
};

#endif // GSETTINGS_H
