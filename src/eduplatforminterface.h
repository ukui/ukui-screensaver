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

#ifndef EDUPLATFORMINTERFACE_H
#define EDUPLATFORMINTERFACE_H
#include <QtDBus/QtDBus>
#include <QImage>
#include <QPixmap>
#include "common.h"
#include <QDebug>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTimer>

class EduPlatformInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char* getInterfaceName()
    {return "cn.kylinos.SSOBackend.eduplatform";}
public:
    EduPlatformInterface(const QString &strService,const QString &strPath,\
                  const QDBusConnection &connection, QObject *parent = 0);

    static EduPlatformInterface* getInstance();

    // 得到登录二维码地址
    DBusMsgCode GetLoginQRUrl(QString &url);
    // 绑定手机号
    DBusMsgCode BindPhoneNum(const QString &username, const QString &phonenum, const QString &vcode);
    // 得到登录二维码
    DBusMsgCode GetLoginQR(QPixmap &qrcode);
    // 得到验证码
    DBusMsgCode GetVerifyCode(const QString &phonenum);
    // 设置Pin码
    DBusMsgCode SetPinCode(const QString &username, const QString &pincode);
    // 检查手机号绑定
    DBusMsgCode CheckPhoneNumBind(const QString &username, bool &isBind);
    // 检查Pin码设置
    DBusMsgCode CheckPincodeSet(const QString &username, bool &isBind);
    // 验证手机号正确性
    DBusMsgCode CheckUserByPhone(const QString &username, const QString &phonenum, const QString &vcode);
    // 通过二维码检查用户身份
    DBusMsgCode CheckUserByQR(QPixmap &qrcode);
    // 通过二维码检查用户身份:获取二维码地址
    DBusMsgCode CheckUserByQR(QString &url);

    // 通过url获取二维码图片数据
    DBusMsgCode getQRCodeFromURL(const QString &url, QPixmap &qrcode);
    
    // 获取用户登录状态
    DBusMsgCode CheckToken(const QString &username);
private:
    void initConnect();
    QPixmap beautifyQRCode(QPixmap &);

    static EduPlatformInterface *m_instance;

Q_SIGNALS:
    void QRStatusChange(QString,QString,int);

private:

};

#endif // EDUPLATFORMINTERFACE_H
