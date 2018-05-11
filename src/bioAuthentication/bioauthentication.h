#ifndef BIOAUTHENTICATION_H
#define BIOAUTHENTICATION_H

#include <QObject>
#include "biocustomtype.h"

class QDBusInterface;

/*!
 * \brief The BioAuthentication class
 * 负责真正的生物识别操作，通过startAuthentication开始认证，
 * 认证完成后会发出携带结果的authenticationComplete信号
 */
class BioAuthentication : public QObject
{
    Q_OBJECT
public:
    explicit BioAuthentication(qint32 uid, const DeviceInfo &deviceInfo, QObject *parent = nullptr);
    void startAuthentication();
    void stopAuthentication();

signals:
    void authenticationComplete(bool result);
    void notify(const QString &message);

private slots:
    void onSearchResult(QDBusPendingCallWatcher *watcher);
    void onStatusChanged(int deviceId, int statusType);

private:
    QDBusInterface      *serviceInterface;

    qint32              uid;
    DeviceInfo          deviceInfo;

};

#endif // BIOAUTHENTICATION_H
