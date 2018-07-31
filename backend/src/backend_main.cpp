#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>
#include "interface.h"
#include "sessionwatcher.h"
#include "interfaceAdaptor.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

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

    SessionWatcher *watcher = new SessionWatcher;
    QObject::connect(watcher, &SessionWatcher::sessionIdle,
                     interface, &Interface::onSessionIdleReceived);


    return a.exec();
}
