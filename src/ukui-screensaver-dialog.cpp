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
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDateTime>
#include <QDebug>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "fullbackgroundwidget.h"

#define CACHE_DIR "/.cache/ukui-screensaver/"

static void
messageOutput(QtMsgType type, const QMessageLogContext &context,const QString &msg);

#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setSetuidAllowed(true);


    //命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Dialog for the ukui ScreenSaver."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption lockOption(QStringLiteral("lock"),
                                  QCoreApplication::translate("main", "lock the screen immediately"));
    QCommandLineOption sessionIdleOption(QStringLiteral("session-idle"),
                                       QCoreApplication::translate("main", "activated by session idle signal"));
    parser.addOptions({lockOption, sessionIdleOption});
    parser.process(a);

    if(!parser.isSet(sessionIdleOption) && !parser.isSet(lockOption))
    {
        return 0;
    }

    qInstallMessageHandler(messageOutput);

    //加载翻译文件
    QString locale = QLocale::system().name();
    QTranslator translator;
    QString qmFile = QString(WORKING_DIRECTORY"/i18n_qm/%1.qm").arg(locale);
    translator.load(qmFile);
    a.installTranslator(&translator);
    qDebug() << "load translation file " << qmFile;

    FullBackgroundWidget *window = new FullBackgroundWidget();

    QFile qssFile(":/qss/assets/authdialog.qss");
    if(qssFile.open(QIODevice::ReadOnly)) {
        a.setStyleSheet(qssFile.readAll());
    }
    qssFile.close();

    if(parser.isSet(lockOption))
    {
        window->lock();
    }

    if(parser.isSet(sessionIdleOption))
    {
        window->onSessionStatusChanged(SESSION_IDLE);
    }

    window->show();
    window->activateWindow();

    return a.exec();
}

static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    QDateTime dateTime = QDateTime::currentDateTime();
    QByteArray time = QString("[%1] ").arg(dateTime.toString("MM-dd hh:mm:ss.zzz")).toLocal8Bit();
    QByteArray localMsg = msg.toLocal8Bit();

    QString filePath(context.file);
    int seprator = filePath.lastIndexOf('/');
    QString fileName = filePath.right(filePath.length() - seprator - 1);
    const char *file = fileName.toLocal8Bit().data();

    switch(type) {
    case QtDebugMsg:
        fprintf(stderr, "%s Debug: %s:%u: %s\n", time.constData(), file, context.line, localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stderr, "%s Info: %s:%u: %s\n", time.constData(), file, context.line, localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "%s Warnning: %s:%u: %s\n", time.constData(), file, context.line, localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s Critical: %s:%u: %s\n", time.constData(), file, context.line, localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s Fatal: %s:%u: %s\n", time.constData(), file, context.line, localMsg.constData());
        abort();
    }
    fflush(stderr);
}
