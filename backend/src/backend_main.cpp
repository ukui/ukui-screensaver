#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>
#include "interface.h"
#include "sessionwatcher.h"
#include "interfaceAdaptor.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 检查该程序是否已经有实例在运行
    QDBusInterface checkInterface("org.freedesktop.DBus",
                                  "/org/freedesktop/DBus",
                                  "org.freedesktop.DBus",
                                  QDBusConnection::sessionBus());
    QDBusReply<bool> ret = checkInterface.call("NameHasOwner",
                                               "cn.kylinos.ScreenSaver");
    if(ret.value()) {
        qDebug() << "There is an instance running";
        exit(EXIT_FAILURE);
    }

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



    return a.exec();
}
