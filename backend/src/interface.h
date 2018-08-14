#ifndef INTERFACE_H
#define INTERFACE_H

#include <QObject>
#include <QDBusContext>
#include <QProcess>


class Interface : public QObject, protected QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "cn.kylinos.ScreenSaver")

public:
    explicit Interface(QObject *parent = nullptr);

Q_SIGNALS:
    void SessionIdle();

public Q_SLOTS:
    /**
     * Lock the screen
     */
    void Lock();

    void onSessionIdleReceived();

    void onNameLost(const QString&);

private:
    void runLocker(bool sessionIdle);
    bool checkExistChild();

private:
    QProcess process;
};

#endif // INTERFACE_H
