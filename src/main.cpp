#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "unixsignallistener.h"
#include "event_monitor.h"
#include <QLabel>

#define CACHE_DIR "/.cache/ukui-screensaver/"

static int setup_unix_signal_handlers();
static void check_exist();
static void redirect(int fd, char *filename);
static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"
int main(int argc, char *argv[])
{
    check_exist();
	setup_unix_signal_handlers();
	QApplication a(argc, argv);
    QApplication::setSetuidAllowed(true);

    UnixSignalListener unixSignalListener;

    qInstallMessageHandler(messageOutput);

    QLocale::Language language;
    language = QLocale::system().language();

    //加载翻译文件
    QTranslator translator;
    if(language == QLocale::Chinese) {
        translator.load(WORKING_DIRECTORY"/i18n_qm/zh_CN.qm");
    }
    a.installTranslator(&translator);
    MainWindow *window = new MainWindow();
    QObject::connect(&unixSignalListener, &UnixSignalListener::transition,
            window, &MainWindow::FSMTransition);

    EventMonitor *monitor = new EventMonitor;
    monitor->start();

    QObject::connect(monitor, &EventMonitor::keyPress, window, &MainWindow::onGlobalKeyPress);
    QObject::connect(monitor, &EventMonitor::buttonDrag, window, &MainWindow::onGlobalMouseMove);

	return a.exec();
}

static int setup_unix_signal_handlers()
{
    struct sigaction usr1, chld;
    int ret = 0;

    usr1.sa_sigaction = UnixSignalListener::usr1SignalAction;
    sigemptyset(&usr1.sa_mask);
    usr1.sa_flags = 0;
    usr1.sa_flags |= SA_SIGINFO;

    chld.sa_sigaction = UnixSignalListener::chldSignalAction;
    sigemptyset(&chld.sa_mask);
    chld.sa_flags = 0;
    chld.sa_flags |= SA_SIGINFO;

    if (sigaction(SIGUSR1, &usr1, 0))
       ret = 1;

    if (sigaction(SIGCHLD, &chld, 0))
       ret = 2;

    return ret;
}

/*
 * check whether there is a process running or not
 */
static void check_exist()
{
    int     fd, val;
    char    buf[16] = {0};
    char    cache_directory[1024] = {0};
    char    pid_file_path[1024] = {0};
    char    log_file_path[1024] = {0};
    QDir    dir;

    snprintf(cache_directory, sizeof(cache_directory), "%s" CACHE_DIR, getenv("HOME"));
    if(!dir.exists(cache_directory)){
        if(!dir.mkdir(cache_directory)){
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
    }
    snprintf(pid_file_path, sizeof(pid_file_path), "%spid", cache_directory);
    if( (fd = open(pid_file_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
        qFatal("open pid file failed: %s", strerror(errno));
    }

    /* try and set a write lock on the pid file */
    struct flock lock;
    lock.l_type     = F_WRLCK;
    lock.l_start    = 0;
    lock.l_whence   = SEEK_SET;
    lock.l_len      = 0;
    if(fcntl(fd, F_SETLK, &lock) < 0) {
        qDebug("there is a process running in this session");
        exit(1);
    }
    /* now, we have the lock */
    if(ftruncate(fd, 0) < 0)
        qFatal("ftruncate error on the pid file: %s", strerror(errno));

    /* write this process ID */
    sprintf(buf, "%d\n", getpid());
    if(write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
        qFatal("write error to the pid file: %s", strerror(errno));

    /* set close-on-exec flag for descriptor */
    if( (val = fcntl(fd, F_GETFD, 0) < 0))
        qFatal("fcntl F_GETFD error: %s", strerror(errno));
    val |= FD_CLOEXEC;
    if(fcntl(fd, F_SETFD, val) < 0)
        qFatal("fcntl F_SETFD error: %s", strerror(errno));

    snprintf(log_file_path, sizeof(log_file_path), "%slog", cache_directory);
    redirect(STDERR_FILENO, log_file_path);
}

static void redirect(int fd, char *filename)
{
    int newfd = open(filename, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if(newfd == -1)
        qFatal("open %s failed: %s", filename, strerror(errno));
    if( dup2(newfd, fd) == -1)
        qFatal("dup2 failed: %s", strerror(errno));
}

static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    QDateTime dateTime = QDateTime::currentDateTime();
    QByteArray time = QString("[%1] ").arg(dateTime.toString("MM-dd hh:mm:ss.zzz")).toLocal8Bit();
    QByteArray localMsg = msg.toLocal8Bit();
    switch(type) {
    case QtDebugMsg:
        fprintf(stderr, "%s Debug: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stderr, "%s Info: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "%s Warnning: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s Critical: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s Fatal: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        abort();
    }
}
