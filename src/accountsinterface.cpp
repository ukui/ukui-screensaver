#include "accountsinterface.h"

const QString STR_EDU_SERVICE = "cn.kylinos.SSOBackend";
const QString STR_EDU_PATH    = "/cn/kylinos/SSOBackend";

AccountsInterface* AccountsInterface::m_instance = nullptr;

AccountsInterface::AccountsInterface(const QString &strService,const QString &strPath,\
              const QDBusConnection &connection, QObject *parent)\
    :QDBusAbstractInterface(strService,strPath,getInterfaceName(),connection,parent)
{

}

AccountsInterface* AccountsInterface::getInstance()
{
    static QMutex mutex;
    mutex.lock();
    if(m_instance == nullptr)
        m_instance = new AccountsInterface(STR_EDU_SERVICE, STR_EDU_PATH, QDBusConnection::systemBus());
    mutex.unlock();
    return m_instance;
}

DBusMsgCode AccountsInterface::SetAccountPincode(const QString &username, const QString &pincode)
{
    QDBusReply<int> reply = call("SetAccountPincode", username, pincode);
    if(!reply.isValid())
    {
        qDebug() << "error: [AccountsInterface][SetAccountPincode]: DBus Connect Failed!";
        return DBusMsgCode::Error_NoReply;
    }
    if(reply.value() != 0)
    {
        qDebug() << "error: [AccountsInterface][SetAccountPincode]: DBus request failed!";
        //return static_cast<DBusMsgCode>(reply.value());
        return DBusMsgCode::Error_NoReply;
    }

    return DBusMsgCode::No_Error;
}

DBusMsgCode AccountsInterface::GetAccountPincode(const QString &username, QString &pincode)
{
    QDBusMessage message = call("GetAccountPincode", username);
    if(QDBusMessage::ErrorMessage == message.type())
    {
        qDebug() << "error: [AccountsInterface][GetAccountPincode]: DBus Connect Failed!";
        return DBusMsgCode::Error_NoReply;
    }
    QList<QVariant> argvs = message.arguments();
    if(argvs.size() != 2)
    {
        qDebug() << "error: [AccountsInterface][GetAccountPincode]: DBus arguments error!";
        return DBusMsgCode::Error_ArgCnt;
    }
    int state = argvs.at(1).value<int>();
    if(state != 0)
    {
        qDebug() << "error: [AccountsInterface][GetAccountPincode]: DBus request failed!";
        //return static_cast<DBusMsgCode>(state);
        return DBusMsgCode::Error_ArgCnt;
    }
    pincode = argvs.at(0).value<QString>();
    qDebug() << "pincode:" << pincode;

    return DBusMsgCode::No_Error;
}

DBusMsgCode AccountsInterface::CheckUserIsNew(const QString &username, bool &isNewUser)
{
    QDBusReply<int> reply = call("CheckUserIsNew", username);
    if(!reply.isValid())
    {
        qDebug() << "info: [AccountsInterface][CheckUserIsNew]: DBus connect failed!";
        return DBusMsgCode::Error_NoReply;
    }
    if(reply.value())
    {
        qDebug() << "info: [AccountsInterface][CheckUserIsNew]: DBus request failed!";
        return DBusMsgCode::Error_UnknownReason;
    }

    isNewUser = static_cast<bool>(reply.value());
    return DBusMsgCode::No_Error;
}

DBusMsgCode AccountsInterface::GetUserPhone(const QString &username, QString &phonenum)
{
    QDBusMessage message = call("GetAccountBasicInfo", username);
    if(QDBusMessage::ErrorMessage == message.type())
    {
        qDebug() << "info: [AccountsInterface][GetUserPhone]: DBus Connect Failed!";
        return DBusMsgCode::Error_NoReply;
    }
    QList<QVariant> argvs = message.arguments();
    if(argvs.size() != 7)
    {
        qDebug() << "info: [AccountsInterface][GetUserPhone]: DBus arguments =" << argvs.size() << "error!";
        return DBusMsgCode::Error_ArgCnt;
    }
    int role = argvs.at(0).value<int>();                Q_UNUSED(role);
    QString school = argvs.at(1).value<QString>();      Q_UNUSED(school);
    QString province = argvs.at(2).value<QString>();    Q_UNUSED(province);
    QString city = argvs.at(3).value<QString>();        Q_UNUSED(city);
    QString county = argvs.at(4).value<QString>();      Q_UNUSED(county);
    phonenum = argvs.at(5).value<QString>();
    int state = argvs.at(6).value<int>();               Q_UNUSED(state);
    return state ? DBusMsgCode::Error_UnknownReason : DBusMsgCode::No_Error;
}
