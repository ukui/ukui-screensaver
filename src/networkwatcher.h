#ifndef NETWORKWATCHER_H
#define NETWORKWATCHER_H
#include <QObject>
#include <QtNetwork>
#include <QDBusInterface>

enum NMState{
    NM_STATE_UNKNOWN           = 0,
    NM_STATE_ASLEEP            = 10,
    NM_STATE_DISCONNECTED      = 20,
    NM_STATE_DISCONNECTING     = 30,
    NM_STATE_CONNECTING        = 40,
    NM_STATE_CONNECTED_LOCAL   = 50,
    NM_STATE_CONNECTED_SITE    = 60,
    NM_STATE_CONNECTED_GLOBAL  = 70,
    NM_STATE_CONNECTED_LIMIT   = 80
};

class NetWorkWatcher : public QObject
{
    Q_OBJECT

public:
    NetWorkWatcher(QObject *parent=nullptr);

Q_SIGNALS:
    void NetworkStateChanged(uint state);

private Q_SLOTS:
    void onNetworkStateChanged(uint state);
    void getIp(QHostInfo info);

public:
    void checkOnline();
    bool isConnect();

private:
    QDBusInterface *network;

};

#endif
