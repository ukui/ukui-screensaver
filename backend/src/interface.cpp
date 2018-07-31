#include "interface.h"
#include <QDebug>
#include <QDBusConnection>
#include <unistd.h>

Interface::Interface(QObject *parent)
    : QObject(parent)
{

}

void Interface::Lock()
{
    qDebug() << "Lock requested";

    if(!checkExistChild())
        runLocker(false);
}

void Interface::onSessionIdleReceived()
{
    Q_EMIT SessionIdle();

    if(!checkExistChild())
        runLocker(true);
}

void Interface::runLocker(bool sessionIdle)
{
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --lock %1").arg(sessionIdle ? "--session-idle" : "");
    qDebug() << cmd;

    process.startDetached(cmd);
}

bool Interface::checkExistChild()
{
    char cmd[128] = {0};
    char str[4];
    FILE *fp;
    int num;

    sprintf(cmd, "ps -aux | grep ukui-screensaver-dialog | grep %s | grep -v grep | wc -l", getenv("USER"));

    fp = popen(cmd, "r");
    fgets(str, sizeof(str)-1, fp);
    pclose(fp);

    num = atoi(str);

    qDebug() << (num > 0 ? "exist dialog running" : "");

    return num > 0;
}
