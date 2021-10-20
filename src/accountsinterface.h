#ifndef ACCOUNTSINTERFACE_H
#define ACCOUNTSINTERFACE_H

#include <QtDBus/QtDBus>
#include <QImage>
#include <QPixmap>
#include <QDebug>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTimer>

#include "common.h"

class AccountsInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char* getInterfaceName()
    {return "cn.kylinos.SSOBackend.accounts";}
public:
    static AccountsInterface* getInstance();

    AccountsInterface(const QString &strService,const QString &strPath,\
                  const QDBusConnection &connection, QObject *parent = 0);

    DBusMsgCode SetAccountPincode(const QString &username, const QString &pincode);

    DBusMsgCode GetAccountPincode(const QString &username, QString &pincode);

    // 检查用户是不是新用户
    DBusMsgCode CheckUserIsNew(const QString &username, bool &isNewUser);
    // 获得用户的手机号
    DBusMsgCode GetUserPhone(const QString &username, QString &phone);
private:
    static AccountsInterface *m_instance;
};

#endif // ACCOUNTSINTERFACE_H
