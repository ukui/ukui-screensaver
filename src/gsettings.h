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
    int getEnum(QString key);

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
