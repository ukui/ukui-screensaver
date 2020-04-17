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

#include "screensaver.h"
#include <QApplication>
#include <QTranslator>
#include <QWindow>
#include <QDebug>
#include <QLabel>
#include <QX11Info>
#include <QtX11Extras>
#include <QVariant>
#include <QCommandLineParser>
#include <X11/Xlib.h>


#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	
    //加载翻译文件
    QString locale = QLocale::system().name();
    QTranslator translator;
    QString qmFile = QString(WORKING_DIRECTORY"/i18n_qm/%1.qm").arg(locale);
    translator.load(qmFile);
    a.installTranslator(&translator);
    qDebug() << "load translation file " << qmFile;


    QCommandLineParser parser;
    QString windowId;
    Screensaver s;
    XWindowAttributes xwa;

    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Screensaver for ukui-screensaver"));
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addOptions({
            {{"r", "root"},
            QCoreApplication::translate("main", "show on root window")},
            {{"w", "window-id"},
            QCoreApplication::translate("main", "show on window."),
            QCoreApplication::translate("main", "window id")},
            });
    parser.process(a);

    bool onWindow = parser.isSet("window-id");
    bool onRoot = parser.isSet("root");

    if(onWindow){
        windowId = parser.value("window-id");
        WId wid =  windowId.toULong();
        QWindow* window = QWindow::fromWinId(wid);
        window->setProperty("_q_embedded_native_parent_handle",QVariant(wid));
        s.winId();
        s.windowHandle()->setParent(window);
        XGetWindowAttributes (QX11Info::display(), wid, &xwa);
        float scale = 1;
        if(!qgetenv("QT_SCALE_FACTOR").isNull())
            scale = qgetenv("QT_SCALE_FACTOR").toFloat();

        s.resize(xwa.width/scale,xwa.height/scale);
        s.move(0,0);
        s.show();
    }
    else if(onRoot){
        WId wid = QX11Info::appRootWindow();
        QWindow* window = QWindow::fromWinId(wid);
        window->setProperty("_q_embedded_native_parent_handle",QVariant(wid));
        s.winId();
        s.windowHandle()->setParent(window);
        XGetWindowAttributes (QX11Info::display(), wid, &xwa);
        qDebug()<<"xwa.width = "<<xwa.width<<"xwa.height = "<<xwa.height;
        s.resize(xwa.width,xwa.height);
        s.move(0,0);
        s.show();
    }
    else{
        s.resize(1366,768);
        s.show();
    }

    return a.exec();
}
