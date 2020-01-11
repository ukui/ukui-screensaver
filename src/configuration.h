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
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

//#include "gsettings.h"

#include <QObject>
#include "screensaver.h"

class QGSettings;

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
    QGSettings *gsettings;
    QGSettings *bgGsettings;
    QString mode;
	QList<QString> themes;
	QString background;
	bool idleActivationEnabled;
	bool lockEnabled;
    int imageTSEffect;
    int imageSwitchInterval;
};

#endif // CONFIGURATION_H
