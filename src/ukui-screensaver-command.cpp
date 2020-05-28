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
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QTranslator>
#include <QCommandLineParser>
#include <QDebug>
#include "types.h"

#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);
	
    QString locale = QLocale::system().name();
    QTranslator translator;
    QString qmFile = QString(WORKING_DIRECTORY"/i18n_qm/%1.qm").arg(locale);
    translator.load(qmFile);
    a.installTranslator(&translator);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Start command for the ukui ScreenSaver."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption lockOption({"l", QStringLiteral("lock")},
                                  QCoreApplication::translate("main", "lock the screen immediately"));

    QCommandLineOption queryOption({"q", QStringLiteral("query")},
                                 QCoreApplication::translate("main", "query the status of the screen saver"));
    parser.addOption(lockOption);
    parser.addOption(queryOption);
    parser.process(a);

    if(!parser.isSet(lockOption)&&!parser.isSet(queryOption))
        return -1;

    QDBusInterface *interface = new QDBusInterface(SS_DBUS_SERVICE,
                                                   SS_DBUS_PATH,
                                                   SS_DBUS_INTERFACE);

    QDBusReply<bool> stateReply = interface->call("GetLockState");
    if(!stateReply.isValid()){
    	qWarning()<< "Get state error:" << stateReply.error();
	return 0;
    }

    if(parser.isSet(queryOption)){
    	if(stateReply)
    		qDebug()<<qPrintable(QObject::tr("The screensaver is active."));
    	else
        	qDebug()<<qPrintable(QObject::tr("The screensaver is inactive."));
    }

    if(parser.isSet(lockOption) && !stateReply){
        QDBusMessage msg = interface->call("Lock");
        if(msg.type() == QDBusMessage::ErrorMessage)
            qDebug() << msg.errorMessage();
    }
    return 0;
}

