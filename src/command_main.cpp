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
#include <QDBusMessage>
#include <QCommandLineParser>
#include <QDebug>


int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Start command for the ukui ScreenSaver."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption lockOption({"l", QStringLiteral("lock")},
                                  QCoreApplication::translate("main", "lock the screen immediately"));
    parser.addOption(lockOption);
    parser.process(a);

    if(!parser.isSet(lockOption))
        return -1;

    QDBusInterface *interface = new QDBusInterface("cn.kylinos.ScreenSaver",
                                                   "/",
                                                   "cn.kylinos.ScreenSaver");
    QDBusMessage msg = interface->call("Lock");
    if(msg.type() == QDBusMessage::ErrorMessage)
        qDebug() << msg.errorMessage();

    return 0;
}

