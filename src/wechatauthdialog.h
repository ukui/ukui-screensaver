/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: ZHAI Kang-ning <zhaikangning@kylinos.cn>
**/
#ifndef WECHATAUTHDIALOG_H
#define WECHATAUTHDIALOG_H

#include <QWidget>
#include <QNetworkReply>
#include <QtNetwork>
#include <QLabel>
#include <QTimer>

#include <common.h>
#include "networkwatcher.h"

class QRLabel;
class RequestQRThread;

/**
 * @brief The WeChatAuthDialog class
 * 微信登录、校验界面
 * 一、标题状态改变:
 *     成功 事例：  1、微信登录->扫描成功->正在登录···
 *     微信不相符： 2、微信登录->登录失败
 */
class WeChatAuthDialog : public QWidget
{
    Q_OBJECT
public:
    explicit WeChatAuthDialog(QWidget *parent = nullptr);
    explicit WeChatAuthDialog(int type, QWidget *parent = nullptr);
    enum authType{
        login = 0,
        check,
    };

    void reloadQR();
    void showErrorMessage(QString message, bool isLong = false);
//    void updateStatus(int status);
    void onReset();

protected:
    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;

Q_SIGNALS:
    void authenticateCompete(bool result);
    void qRStatusChange(QString loginname, QString loginpwd, int curstatus);

private:
    void initUI();
    void initConnect();

    int m_authType = 0;
    QNetworkAccessManager *m_networkManager;
    QRLabel *m_loginQR;
    QLabel  *m_title; //标题
    QLabel  *m_guide; //提示
    QLabel  *m_errorMsg; //错误信息
    RequestQRThread *m_requestQRThread = nullptr;
    bool m_isThreadWorking = false;
    NetWorkWatcher *m_networkWatcher;
    QTimer *m_timer;

    void getLoginQR();

private Q_SLOTS:
    void replyFinished(QNetworkReply *);
    void qRStatusChangeSolt(QString loginname, QString loginpwd, int curstatus);
    void onQRUrlReady(int errorCode, const QString url);
    void onNetworkStateChanged(uint state);
};

class QRLabel : public QLabel
{
    Q_OBJECT
public:
    explicit QRLabel(QWidget *parent = nullptr);

Q_SIGNALS:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

class RequestQRThread : public QThread
{
    Q_OBJECT
public:
    RequestQRThread(QObject *parent);

protected:
    void run() override;

Q_SIGNALS:
    void resultReady(int errorCode, const QString url);
};

#endif // WECHATAUTHDIALOG_H
