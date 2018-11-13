#include "displaymanager.h"
#include <QDBusInterface>
#include <QDebug>
#include <QDBusMessage>
#include <QDBusArgument>
#include "types.h"

DisplayManager::DisplayManager(QObject *parent) : QObject(parent)
{
    char *seatPath = getenv("XDG_SEAT_PATH");
    qDebug() << seatPath;
    dmService = new QDBusInterface(DM_DBUS_SERVICE,
                                   seatPath,
                                   DBUS_PROP_INTERFACE,
                                   QDBusConnection::systemBus());
    dmSeatService = new QDBusInterface(DM_DBUS_SERVICE,
                                       seatPath,
                                       DM_SEAT_INTERFACE,
                                       QDBusConnection::systemBus());
    getProperties();
}

bool DisplayManager::canSwitch()
{
    return _canSwitch;
}

bool DisplayManager::hasGuestAccount()
{
    return _hasGuestAccount;
}

void DisplayManager::switchToGreeter()
{
    QDBusMessage ret = dmSeatService->call("SwitchToGreeter");

    handleDBusError(ret);
}

void DisplayManager::switchToUser(const QString &userName)
{
    QDBusMessage ret = dmSeatService->call("SwitchToUser", userName, "");

    handleDBusError(ret);
}

void DisplayManager::switchToGuest()
{
    QDBusMessage ret = dmSeatService->call("SwitchToGuest", "");

    handleDBusError(ret);
}

void DisplayManager::getProperties()
{
    QDBusMessage ret = dmService->call("GetAll", DM_SEAT_INTERFACE);
    handleDBusError(ret);
    const QDBusArgument &arg = ret.arguments().at(0).value<QDBusArgument>();
    qDebug() << arg.currentType();

    arg.beginMap();
    while(!arg.atEnd())
    {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        arg.endMapEntry();
        if(key == "CanSwitch")
        {
            _canSwitch = value.toBool();
        }
        else if(key == "HasGuestAccount")
        {
            _hasGuestAccount = value.toBool();
        }
    }
    arg.endMap();
}

void DisplayManager::handleDBusError(const QDBusMessage &msg)
{
    if(msg.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << msg.errorMessage();
    }
}
