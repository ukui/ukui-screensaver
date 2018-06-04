#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "unixsignallistener.h"

#define PID_FILE "/tmp/ukui-screensaver.pid"

static int setup_unix_signal_handlers();
static void check_exist();

#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"
int main(int argc, char *argv[])
{
    check_exist();

	setup_unix_signal_handlers();
	QApplication a(argc, argv);
	UnixSignalListener unixSignalListener;

	QLocale::Language language;
    language = QLocale::system().language();

    //加载翻译文件
    QTranslator translator;
    if(language == QLocale::Chinese) {
        translator.load(WORKING_DIRECTORY"/i18n_qm/zh_CN.qm");
    }
    a.installTranslator(&translator);
	MainWindow w;
	QObject::connect(&unixSignalListener, &UnixSignalListener::transition,
			&w, &MainWindow::FSMTransition);

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
    if( (fd = open(PID_FILE, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) < 0)
        qFatal("open pid file failed: %s", strerror(errno));

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

}
