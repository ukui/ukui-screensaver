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
#include <syslog.h>
#include <QDBusReply>
#include <QLocale>
#include <QDir>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDateTime>
#include <QWidget>
#include <QDebug>
#include <QDesktopWidget>
#include <QDBusInterface>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <QProcess>
#include <X11/Xlib.h>
#include "fullbackgroundwidget.h"
#include "configuration.h"
#define CACHE_DIR "/.cache/ukui-screensaver/"
#define DOUBLE 2
#define MAX_FILE_SIZE 1024 * 1024
#define LOG_FILE0 "screensaver_0.log"
#define LOG_FILE1 "screensaver_1.log"

#define GSETTINGS_SCHEMA_SCREENSAVER "org.ukui.screensaver"
#define KEY_LOCK_ENABLED "lock-enabled"

FullBackgroundWidget *window = NULL;
static void
messageOutput(QtMsgType type, const QMessageLogContext &context,const QString &msg);

void checkIslivecd()
{
    char cmd[128] = {0};
    char str[1024];
    FILE *fp;

    int n = sprintf(cmd, "cat /proc/cmdline");
    Q_UNUSED(n)

    fp = popen(cmd, "r");
    while(fgets(str, sizeof(str)-1, fp)) {
        if(strstr(str,"boot=casper"))
        {
                printf("is livecd\n");
                exit(0);
        }
    }
    pclose(fp);

    QString filepath = QDir::homePath() + "/Desktop" + "/kylin-os-installer.desktop";
    QFileInfo file(filepath);
    if(!file.exists())
            return;
    if(getuid() != 999)
            return;
    exit(0);
}

void checkIsRunning()
{
    int fd, len;
    char buf[32];
    struct flock lock;

    const QString PID_DIR = QString("/var/run/user/%1").arg(QString::number(getuid()));
    QString env = qgetenv("DISPLAY");
    const QString PID_FILE = PID_DIR + QString("/ukui-screensaver%1.pid").arg(env);

    qDebug() << PID_DIR;
    QDir dir(PID_DIR);
    if(!dir.exists()) {
        if(!dir.mkdir(PID_DIR.toLocal8Bit().data())) {
            perror("create pid directory failed");
            exit(1);
        }
    }
    if( (fd = open(PID_FILE.toLocal8Bit().data(),
                   O_RDWR | O_CREAT, 0666)) == -1){
        perror("open pid file failed");
        exit(1);
    }

    memset(&lock, 0, sizeof(struct flock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;

    if(fcntl(fd, F_SETLK, &lock) < 0) {
//        perror("fcntl F_SETLK failed");
        printf("There is already an instance running\n");
        exit(1);
    }

    len = snprintf(buf, sizeof(buf), "%d", getpid());
    ftruncate(fd, 0);
    if(write(fd, buf, len) != len) {
        perror("write pid to lock file failed");
        exit(1);
    }
}

void handler(int signum)
{
    window->closeScreensaver();
}

#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"
int main(int argc, char *argv[])
{
    if(argc < 2)
	return 0;

    checkIsRunning();
    checkIslivecd();
 
    if(QString(argv[1]) == "--lock-startup"){
        Configuration::instance();
        QDBusInterface *checkInterface =
                new QDBusInterface("org.freedesktop.DBus",
                                   "/org/freedesktop/DBus",
                                   "org.freedesktop.DBus",
                                   QDBusConnection::sessionBus());
        for(int i = 0;i<20;i++){
            QDBusReply<bool> ret = checkInterface->call("NameHasOwner",
                                                        "org.gnome.SessionManager");
            if(ret.value()) {
                break;
            }

            usleep(100000);
        }
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
 
    qunsetenv("QT_IM_MODULE");
    //signal(SIGTERM,handler);
    QApplication a(argc, argv);
    QApplication::setSetuidAllowed(true);

    QDesktopWidget *desktop = QApplication::desktop();
    if(desktop->geometry().width()<=0 || desktop->geometry().height()<=0)
        return 0;

    //命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Dialog for the ukui ScreenSaver."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption lockOption(QStringLiteral("lock"),
                                  QCoreApplication::translate("main", "lock the screen immediately"));
    QCommandLineOption lstOption(QStringLiteral("lock-startup"),
                                  QCoreApplication::translate("main", "lock the screen immediately"));
    QCommandLineOption sessionIdleOption(QStringLiteral("session-idle"),
                                       QCoreApplication::translate("main", "activated by session idle signal"));
    QCommandLineOption lscreensaverOption(QStringLiteral("lock-screensaver"),
                                       QCoreApplication::translate("main", "lock the screen and show screensaver immediately"));
    QCommandLineOption screensaverOption(QStringLiteral("screensaver"),
                                       QCoreApplication::translate("main", "show screensaver immediately"));
    QCommandLineOption blankOption(QStringLiteral("blank"),
                                       QCoreApplication::translate("main", "lock the screen and show screensaver immediately"));
    parser.addOptions({lockOption, lstOption,sessionIdleOption , screensaverOption,blankOption,lscreensaverOption});
    parser.process(a);

    if(!parser.isSet(sessionIdleOption) 
		   && !parser.isSet(lockOption) 
		   && !parser.isSet(lstOption) 
		   && !parser.isSet(screensaverOption) 
		   && !parser.isSet(lscreensaverOption)
		   && !parser.isSet(blankOption))
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

    window = new FullBackgroundWidget();

    QFile qssFile(":/qss/assets/authdialog.qss");
    if(qssFile.open(QIODevice::ReadOnly)) {
        a.setStyleSheet(qssFile.readAll());
    }
    qssFile.close();

    if(parser.isSet(blankOption))
    {
        window->onBlankScreensaver();
    }

#ifndef USE_INTEL
    window->show();
    window->activateWindow();
#endif
    if(parser.isSet(lockOption))
    {
        window->lock();
    }
    
    if(parser.isSet(lstOption))
    {
        window->lock();
	window->setIsStartup(true);
    }

    if(parser.isSet(sessionIdleOption))
    {
        if(window->onSessionStatusChanged(SESSION_IDLE) == -1)
	    return 0;
    }
 
    if(parser.isSet(lscreensaverOption))
    {
    	window->onScreensaver();
    }
/*    
    if(parser.isSet(blankOption))
    {
        window->onBlankScreensaver();
    }
*/
    if(parser.isSet(screensaverOption))
    {
    	window->showScreensaver();
    }

#ifdef USE_INTEL
    window->show();
    window->activateWindow();
#endif

    QString username = getenv("USER");
    int uid = getuid();
    QDBusInterface *interface = new QDBusInterface("cn.kylinos.Kydroid2",
                                                   "/cn/kylinos/Kydroid2",
                                                   "cn.kylinos.Kydroid2",
                                                   QDBusConnection::systemBus(),
                                                   window);

    QDBusMessage msg = interface->call(QStringLiteral("SetPropOfContainer"),username, uid, "is_kydroid_on_focus", "0");
    return a.exec();
}

static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    QDateTime dateTime = QDateTime::currentDateTime();
    QByteArray time = QString("[%1] ").arg(dateTime.toString("MM-dd hh:mm:ss.zzz")).toLocal8Bit();
    QByteArray localMsg = msg.toLocal8Bit();

    QString name[DOUBLE] = {LOG_FILE0, LOG_FILE1};
    FILE *log_file = nullptr;
    QString logFilePath;
    int fileSize;
    static int i = 0;
    QDir dir;

    if (dir.mkpath(QDir::homePath() + CACHE_DIR)) {
        logFilePath = QDir::homePath() + CACHE_DIR + "/" + name[i];
        log_file = fopen(logFilePath.toLocal8Bit().constData(), "a+");
    }

    QString filePath(context.file);
    int separator = filePath.lastIndexOf('/');
    QString fileName = filePath.right(filePath.length() - separator - 1);
    const char *file = fileName.toLocal8Bit().data();

    switch(type) {
    case QtDebugMsg:
        fprintf(log_file? log_file:stderr, "%s Debug: %s:%u: %s\n", time.constData(), file, context.line, localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(log_file? log_file:stderr, "%s Info: %s:%u: %s\n", time.constData(), file, context.line, localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(log_file? log_file:stderr, "%s Warnning: %s:%u: %s\n", time.constData(), file, context.line, localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(log_file? log_file:stderr, "%s Critical: %s:%u: %s\n", time.constData(), file, context.line, localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(log_file? log_file:stderr, "%s Fatal: %s:%u: %s\n", time.constData(), file, context.line, localMsg.constData());
        abort();
    }
    fflush(stderr);

    if (log_file) {
        fileSize = ftell(log_file);
        if (fileSize >= MAX_FILE_SIZE) {
            i = (i + 1) % DOUBLE;
            logFilePath = QDir::homePath() + logFilePath + "/" + name[i];
            if (QFile::exists(logFilePath)) {
                QFile temp(logFilePath);
                temp.remove();
            }
        }
        fclose(log_file);
    }
}
