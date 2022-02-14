/**
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/

#include "eduplatforminterface.h"

const QString STR_EDU_SERVICE = "cn.kylinos.SSOBackend";
const QString STR_EDU_PATH    = "/cn/kylinos/SSOBackend";

EduPlatformInterface* EduPlatformInterface::m_instance = nullptr;

EduPlatformInterface::EduPlatformInterface(const QString &strService,const QString &strPath,\
              const QDBusConnection &connection, QObject *parent)\
    :QDBusAbstractInterface(strService,strPath,getInterfaceName(),connection,parent)
{
    initConnect();
}

EduPlatformInterface* EduPlatformInterface::getInstance()
{
    static QMutex mutex;
    mutex.lock();
    if(m_instance == nullptr || !m_instance->isValid())
        m_instance = new EduPlatformInterface(STR_EDU_SERVICE, STR_EDU_PATH, QDBusConnection::systemBus());
    mutex.unlock();
    return m_instance;
}

void EduPlatformInterface::initConnect()
{

}

DBusMsgCode EduPlatformInterface::GetLoginQR(QPixmap &qrcode)
{
    QDBusMessage message = call("GetLoginQR");
    if(QDBusMessage::ErrorMessage == message.type())
    {
        qDebug() << "info: [EduPlatformInterface][GetLoginQR]: DBus Connect Failed!";
        return DBusMsgCode::Error_NoReply;
    }
    QList<QVariant> argvs = message.arguments();
    if(argvs.size() != 2)
    {
        qDebug() << "info: [EduPlatformInterface][GetLoginQR]: DBus arguments error!";
        return DBusMsgCode::Error_ArgCnt;
    }
    int state = argvs.at(1).value<int>();
    if(state != 0)
    {
        qDebug() << "info: [EduPlatformInterface][GetLoginQR]: DBus request failed!";
        //return static_cast<DBusMsgCode>(state);
        return DBusMsgCode::Error_ArgCnt;
    }
    const QString url = argvs.at(0).value<QString>();

    DBusMsgCode download_errcode = getQRCodeFromURL(url, qrcode);
    if(download_errcode != DBusMsgCode::No_Error) return download_errcode;
    qrcode = beautifyQRCode(qrcode);
    return DBusMsgCode::No_Error;
}

DBusMsgCode EduPlatformInterface::GetLoginQRUrl(QString &url)
{
    QDBusMessage message = call("GetLoginQR");
    if(QDBusMessage::ErrorMessage == message.type())
    {
        qDebug() << "info: [EduPlatformInterface][GetLoginQR]: DBus Connect Failed!";
        return DBusMsgCode::Error_NoReply;
    }
    QList<QVariant> argvs = message.arguments();
    if(argvs.size() != 2)
    {
        qDebug() << "info: [EduPlatformInterface][GetLoginQR]: DBus arguments error!";
        return DBusMsgCode::Error_ArgCnt;
    }
    int state = argvs.at(1).value<int>();
    if(state != 0)
    {
        qDebug() << "info: [EduPlatformInterface][GetLoginQR]: DBus request failed!";
        //return static_cast<DBusMsgCode>(state);
        return DBusMsgCode::Error_ArgCnt;
    }
    url = argvs.at(0).value<QString>();
    return DBusMsgCode::No_Error;
}

DBusMsgCode EduPlatformInterface::BindPhoneNum(const QString &username, const QString &phonenum, const QString &vcode)
{
    QDBusReply<int> reply = call("BindPhoneNum", username, phonenum, vcode);
    if(!reply.isValid())
    {
        qDebug() << "info: [EduPlatformInterface][BindPhoneNum]: DBus Connect Failed!";
        return DBusMsgCode::Error_NoReply;
    }
    if(reply.value() != 0)
    {
        qDebug() << "info: [EduPlatformInterface][BindPhoneNum]: DBus request failed!";
        //return static_cast<DBusMsgCode>(reply.value());
        return DBusMsgCode::Error_NoReply;
    }

    return DBusMsgCode::No_Error;
}

DBusMsgCode EduPlatformInterface::GetVerifyCode(const QString &phonenum)
{
    QDBusReply<int> reply = call("GetVerifyCode", phonenum);
    if(!reply.isValid())
    {
        qDebug() << "info: [EduPlatformInterface][GetVerifyCode]: DBus Connect Failed!";
        return DBusMsgCode::Error_NoReply;
    }
    //
    if(reply.value() != 0) // 默认0为成功
    {
        qDebug() << "info: [EduPlatformInterface][GetVerifyCode]: DBus request failed!";
        //return static_cast<DBusMsgCode>(reply.value());
        return DBusMsgCode::Error_NoReply;
    }

    return DBusMsgCode::No_Error;
}

DBusMsgCode EduPlatformInterface::SetPinCode(const QString &username, const QString &pincode)
{
    QDBusReply<int> reply = call("SetPinCode", username, pincode);
    if(!reply.isValid())
    {
        qDebug() << "info: [][]: DBus connect failed!";
        return DBusMsgCode::Error_NoReply;
    }
    if(!reply.value())
    {
        return DBusMsgCode::No_Error;
    }
    return DBusMsgCode::Error_NoReply;
}

DBusMsgCode EduPlatformInterface::CheckToken(const QString &username)
{
    QDBusReply<int> reply = call("CheckToken", username);
    if(!reply.isValid())
    {
        qDebug() << "info: [EduPlatformInterface][RefreshToken]: DBus connect failed!";
        return DBusMsgCode::Error_NoReply;
    }
//    if(reply.value())
//    {
//        qDebug() << "info: [EduPlatformInterface][RefreshToken]: DBus return-result failed!";
//    }
    switch (reply.value()) {
    case 0: return DBusMsgCode::No_Error;
    default: return static_cast<DBusMsgCode>(reply.value());
    }
}

DBusMsgCode EduPlatformInterface::CheckPhoneNumBind(const QString &username, bool &isBind)
{
    QDBusReply<int> reply = call("CheckPhoneNumBind", username);
    if(!reply.isValid())
    {
        qDebug() << "info: [EduPlatformInterface][CheckPhoneNumBind]: DBus connect failed!";
        return DBusMsgCode::Error_NoReply;
    }
    // TODO 需要改接口 接口有问题
    if(reply.value() == 0)
    {
        isBind = false;
        return DBusMsgCode::No_Error;
    }
    if(reply.value() == 1)
    {
        isBind = true;
        return DBusMsgCode::No_Error;
    }
    isBind = true;
    return DBusMsgCode::Error_NoReply;
}

DBusMsgCode EduPlatformInterface::CheckPincodeSet(const QString &username, bool &isBind)
{
    QDBusReply<int> reply = call("CheckPincodeSet", username);
    if(!reply.isValid())
    {
        qDebug() << "info: [EduPlatformInterface][CheckPincodeSet]: DBus connect failed!";
        return DBusMsgCode::Error_NoReply;
    }

    if(reply.value() == 0)
    {
        isBind = true;
        return DBusMsgCode::No_Error;
    }
    if(reply.value() == 1)
    {
        isBind = false;
        return DBusMsgCode::No_Error;
    }

    return DBusMsgCode::Error_NoReply;
}

DBusMsgCode EduPlatformInterface::CheckUserByPhone(const QString &username, const QString &phonenum, const QString &vcode)
{
    QDBusReply<int> reply = call("CheckUserByPhone", username, phonenum, vcode);
    if(!reply.isValid())
    {
        qDebug() << "info: [EduPlatformInterface][CheckUserByPhone]: DBus connect failed!";
        return DBusMsgCode::Error_NoReply;
    }
    return static_cast<DBusMsgCode>(reply.value());
}

DBusMsgCode EduPlatformInterface::CheckUserByQR(QPixmap &qrcode)
{
    QDBusMessage message = call("CheckUserByQR");
    if(QDBusMessage::ErrorMessage == message.type())
    {
        qDebug() << "info: [EduPlatformInterface][CheckUserByQR]: DBus Connect Failed!";
        return DBusMsgCode::Error_NoReply;
    }
    QList<QVariant> argvs = message.arguments();
    if(argvs.size() != 2)
    {
        qDebug() << "info: [EduPlatformInterface][CheckUserByQR]: DBus arguments error!";
        return DBusMsgCode::Error_ArgCnt;
    }
    int state = argvs.at(1).value<int>();
    if(state != 0)
    {
        qDebug() << "info: [EduPlatformInterface][CheckUserByQR]: DBus request failed!";
        //return static_cast<DBusMsgCode>(state);
        return static_cast<DBusMsgCode>(state);
    }
    const QString url = argvs.at(0).value<QString>();

    DBusMsgCode download_errcode = getQRCodeFromURL(url, qrcode);
    if(download_errcode != DBusMsgCode::No_Error) return download_errcode;
    qrcode = beautifyQRCode(qrcode);
    return DBusMsgCode::No_Error;
}

DBusMsgCode EduPlatformInterface::CheckUserByQR(QString &url)
{
    QDBusMessage message = call("CheckUserByQR");
    if(QDBusMessage::ErrorMessage == message.type())
    {
        qDebug() << "info: [EduPlatformInterface][CheckUserByQR]: DBus Connect Failed!";
        return DBusMsgCode::Error_NoReply;
    }
    QList<QVariant> argvs = message.arguments();
    if(argvs.size() != 2)
    {
        qDebug() << "info: [EduPlatformInterface][CheckUserByQR]: DBus arguments error!";
        return DBusMsgCode::Error_ArgCnt;
    }
    int state = argvs.at(1).value<int>();
    if(state != 0)
    {
        qDebug() << "info: [EduPlatformInterface][CheckUserByQR]: DBus request failed!";
        //return static_cast<DBusMsgCode>(state);
        return static_cast<DBusMsgCode>(state);
    }
    url = argvs.at(0).value<QString>();
    return DBusMsgCode::No_Error;
}

DBusMsgCode EduPlatformInterface::getQRCodeFromURL(const QString &url, QPixmap &qrcode)
{
    // 从网络上下载二维码   并加载到qrcode
    QNetworkRequest request;
    QNetworkAccessManager networkManager;
    request.setUrl(url);
    QNetworkReply *reply = networkManager.get(request);

    // 超时 \ 同步处理
    QEventLoop loop;
    QTimer timer;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &timer, &QTimer::stop);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(3000);
    loop.exec();

    if(!timer.isActive())
    {
        reply->abort();
        qDebug() << "info: [EduPlatformInterface][GetLoginQR]: Get QRCode timeout!";
        return DBusMsgCode::Error_QR_Get_Timeout;
    }
    timer.stop();

    // 生成二维码
    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "info: [EduPlatformInterface][GetLoginQR]: Network error!" << reply->errorString();
        return DBusMsgCode::Error_NetWork;
    }
    QByteArray bytes = reply->readAll();
    qrcode.loadFromData(bytes);
    return DBusMsgCode::No_Error;
}

// 美化图片
QPixmap EduPlatformInterface::beautifyQRCode(QPixmap &pixImg)
{
    // 去除白边
    QImage img = pixImg.toImage();
    int rect_x, rect_y, rect_width, rect_height;
    for(int i = 0;i < img.size().width();i++)
    {
        bool isEnd = false;
        for(int j = 0;j < img.size().height();j++)
        {
            if(img.pixel(i, j) != qRgb(255, 255, 255))
            {
                rect_x = i - 1;
                rect_y = j - 1;
                rect_width = img.size().width() - 2 * rect_x;
                rect_height = img.size().height() - 2 * rect_y;
                isEnd = true;
                break;
            }
        }
        if(isEnd) break;
    }
    QPixmap dealImg = QPixmap::fromImage(img.copy(rect_x, rect_y, rect_width, rect_height)).scaledToWidth(150);
    img = dealImg.toImage();

    // 添加微信log
//    QPixmap piximg_wechat(WECHAT_LOG_PATH);
//    QImage img_wechat = piximg_wechat.scaledToWidth(150).toImage();
//    for(int i = 0;i < img.size().width();++i)
//    {
//        for(int j = 0;j < img.size().height(); ++j)
//        {
//            if(img.pixel(i, j) > qRgb(125, 125, 125))
//            {
//                int r = qRed(img_wechat.pixel(i, j));
//                int g = qGreen(img_wechat.pixel(i, j));
//                int b = qBlue(img_wechat.pixel(i, j));
//                int alpha = qAlpha(img_wechat.pixel(i, j));
//                if(alpha == 0) r = 255, b = 255, g = 255;
//                else r = 0x25, g = 0xAC, b = 0x36;
//                img.setPixelColor(i, j, QColor(r, g, b, alpha));
//                //img.setPixel(i, j, qRgb(0, 255, 0));
//            }
//        }
//    }
    dealImg = QPixmap::fromImage(img);
    return dealImg;
}

//void EduPlatformInterface::onScanState(QString username, QString password, int state)
//{
//    switch(state)
//    {
//    case 4: // 扫码并确认完成
//        qDebug() << "=================扫码并确认完成" << username << password;
//        emit qrcodeScanState(QRCodeSwepState::ConfirmSuccess, username, password);
//        break;
//    case 2: // 用户扫码
//        qDebug() << "=================用户扫码" << username << password;
//        emit qrcodeScanState(QRCodeSwepState::HaveSwep, "", "");
//        break;
//    case 3: // 用户取消扫码
//        qDebug() << "=================用户取消扫码" << username << password;
//        emit qrcodeScanState(QRCodeSwepState::CancelSwep, "", "");
//        break;
//    case 5: // 二维码超时
//        qDebug() << "=================二维码超时" << username << password;
//        emit qrcodeScanState(QRCodeSwepState::QRCodeInvalid, "", "");
//        break;
//    default:
//        break;
//    }
//}

