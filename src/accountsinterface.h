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
