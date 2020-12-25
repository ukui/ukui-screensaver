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
#include <QDesktopWidget>
#include <QDBusInterface>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <QProcess>
#include <X11/Xlib.h>
#include "fullbackgroundwidget.h"

#define CACHE_DIR "/.cache/ukui-screensaver/"

static void
messageOutput(QtMsgType type, const QMessageLogContext &context,const QString &msg);

void checkIslivecd()
{
    char cmd[128] = {0};
    char str[1024];
    FILE *fp;
    int pid;

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

    QString filepath = QDir::homePath() + "/Desktop" + "/ubiquity.desktop";
    QString folderpath = "/cdrom";
    QFileInfo file(filepath);
    QFileInfo folder(folderpath);
    if(!file.exists())
            return;
    if(!folder.exists())
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
    qApp->quit();
}

#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"
int main(int argc, char *argv[])
{
    checkIsRunning();
    checkIslivecd();
	
#if(QT_VERSION>=QT_VERSION_CHECK(5,6,0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    qunsetenv("QT_IM_MODULE");
//    signal(SIGTERM,handler);
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
    QCommandLineOption screensaverOption(QStringLiteral("screensaver"),
                                       QCoreApplication::translate("main", "lock the screen and show screensaver immediately"));
    QCommandLineOption blankOption(QStringLiteral("blank"),
                                       QCoreApplication::translate("main", "lock the screen and show screensaver immediately"));
    parser.addOptions({lockOption, sessionIdleOption , screensaverOption,blankOption});
    parser.process(a);

    if(!parser.isSet(sessionIdleOption) && !parser.isSet(lockOption) && !parser.isSet(screensaverOption) && !parser.isSet(blankOption))
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
        if(window->onSessionStatusChanged(SESSION_IDLE) == -1)
	    return 0;
    }
 
    if(parser.isSet(screensaverOption))
    {
    	window->onScreensaver();
    }
    
    if(parser.isSet(blankOption))
    {
        window->onBlankScreensaver();
    }

    window->show();
    window->activateWindow();
	
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

    QString filePath(context.file);
    int separator = filePath.lastIndexOf('/');
    QString fileName = filePath.right(filePath.length() - separator - 1);
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
