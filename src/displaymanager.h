#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <QObject>

class QDBusInterface;
class QDBusMessage;

class DisplayManager : public QObject
{
    Q_OBJECT
public:
    explicit DisplayManager(QObject *parent = nullptr);
    void switchToGreeter();
    void switchToUser(const QString &userName);
    void switchToGuest();
    bool canSwitch();
    bool hasGuestAccount();

private:
    void getProperties();
    void handleDBusError(const QDBusMessage &msg);

private:
    bool _canSwitch;
    bool _hasGuestAccount;

    QDBusInterface *dmService;
    QDBusInterface *dmSeatService;
};

#endif // DISPLAYMANAGER_H
