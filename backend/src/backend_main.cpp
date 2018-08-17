#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>
#include "interface.h"
#include "sessionwatcher.h"
#include "interfaceAdaptor.h"
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

void sig_chld(int /*signo*/)
{
    pid_t pid;
    while( (pid = waitpid(-1, NULL, WNOHANG)) > 0)
        qDebug() << "child" << pid << "terminated";
    return;
}

int main(int argc, char *argv[])
{
    if(signal(SIGCHLD, sig_chld) == SIG_ERR) {
        perror("signal error");
        exit(EXIT_FAILURE);
    }

    QCoreApplication a(argc, argv);

    // 检查该程序是否已经有实例在运行
    QDBusInterface *checkInterface =
            new QDBusInterface("org.freedesktop.DBus",
                               "/org/freedesktop/DBus",
                               "org.freedesktop.DBus",
                               QDBusConnection::sessionBus());
    QDBusReply<bool> ret = checkInterface->call("NameHasOwner",
                                               "cn.kylinos.ScreenSaver");
    if(ret.value()) {
        qDebug() << "There is an instance running";
        exit(EXIT_FAILURE);
    }

    // 如果已经有实例在运行则kill, 主要是针对注销后重新登录时之前的实例没有被kill掉
    char cmd[128] = {0};
    char str[16];
    FILE *fp;
    int pid;

    sprintf(cmd, "ps aux | grep ukui-screensaver-backend | grep %s | grep -v grep | awk '{print $2}'", getenv("USER"));

    fp = popen(cmd, "r");
    while(fgets(str, sizeof(str)-1, fp)) {
        pid = atoi(str);

        if(pid > 0 && pid != getpid()) {
            qDebug() << "existing instance pid: " << pid;
            kill(pid, SIGKILL);
        }
    }
    pclose(fp);


    // 注册DBus
    Interface *interface = new Interface();
    ScreenSaverAdaptor adaptor(interface);

    QDBusConnection service = QDBusConnection::sessionBus();
    if(!service.registerService("cn.kylinos.ScreenSaver")) {
        qDebug() << service.lastError().message();
        exit(EXIT_FAILURE);
    }
    if(!service.registerObject("/", "cn.kylinos.ScreenSaver", &adaptor,
                               QDBusConnection::ExportAllSlots |
                               QDBusConnection::ExportAllSignals)) {
        qDebug() << service.lastError().message();
        exit(EXIT_FAILURE);
    }
    qDebug() << service.baseService();

    // 发送DBus信号
    SessionWatcher *watcher = new SessionWatcher;
    QObject::connect(watcher, &SessionWatcher::sessionIdle,
                     interface, &Interface::onSessionIdleReceived);
    QObject::connect(watcher, &SessionWatcher::sessionIdle,
                     &a, [&]{
        QDBusMessage message = QDBusMessage::createSignal("/",
                                                          "cn.kylinos.ScreenSaver",
                                                          "SessionIdle");
        service.send(message);
    });


    QObject::connect(checkInterface, SIGNAL(NameLost(QString)),
                     interface, SLOT(onNameLost(QString)));


    return a.exec();
}
