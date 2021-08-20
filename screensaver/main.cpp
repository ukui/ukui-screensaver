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
#include <X11/Xutil.h>
#include <sys/prctl.h>
#include <signal.h>
#include <syslog.h>

#include "config.h"

#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"
bool bControlFlg = false;//是否控制面板窗口

int main(int argc, char *argv[])
{
#if(QT_VERSION>=QT_VERSION_CHECK(5,6,0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication a(argc, argv);
    prctl(PR_SET_PDEATHSIG, SIGHUP);	
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

    double scale = 1;
    QScreen *screen = QApplication::primaryScreen();
    scale = screen->devicePixelRatio();

    if(onWindow){
        windowId = parser.value("window-id");
        WId wid =  windowId.toULong();
        QWindow* window = QWindow::fromWinId(wid);
        window->setProperty("_q_embedded_native_parent_handle",QVariant(wid));
        s.winId();
        s.windowHandle()->setParent(window);
        XGetWindowAttributes (QX11Info::display(), wid, &xwa);

#ifndef USE_INTEL
        XClassHint ch;
        ch.res_name = NULL;
        ch.res_class = NULL;
        XGetClassHint (QX11Info::display(), wid, &ch);
        if(ch.res_name && strcmp(ch.res_name,"ukui-control-center")==0){
            bControlFlg = true;
            s.addClickedEvent();
        }
#endif

        //获取屏保所在屏幕对应的缩放比例。
        for(auto screen : QGuiApplication::screens())
        {
            QPoint pos(xwa.x,xwa.y);
            if(screen->geometry().contains(pos)){
                scale = screen->devicePixelRatio();
            }
        }

        s.resize(xwa.width/scale + 1,xwa.height/scale + 1);
        s.move(0,0);
        s.show();
    }
    else if(onRoot){
        bControlFlg = false;
        WId wid = QX11Info::appRootWindow();
        QWindow* window = QWindow::fromWinId(wid);
        window->setProperty("_q_embedded_native_parent_handle",QVariant(wid));
        s.winId();
        s.windowHandle()->setParent(window);
        XGetWindowAttributes (QX11Info::display(), wid, &xwa);
        qDebug()<<"xwa.width = "<<xwa.width<<"xwa.height = "<<xwa.height;
        s.resize(xwa.width/scale + 1,xwa.height/scale + 1);
        s.move(0,0);
        s.show();
    }
    else{
        s.resize(1366,768);
        s.show();
    }

    return a.exec();
}
