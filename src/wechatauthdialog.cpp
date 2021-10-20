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
#include "wechatauthdialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QDBusInterface>
#include <QDBusReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QMouseEvent>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QBitmap>
#include <QPainter>
#include "eduplatforminterface.h"

WeChatAuthDialog::WeChatAuthDialog(QWidget *parent) : QWidget(parent),
    m_networkManager(new QNetworkAccessManager(this)),
    m_loginQR(new QRLabel(this)),
    m_title(new QLabel(this)),
    m_guide(new QLabel(this)),
    m_errorMsg(new QLabel(this)),
    m_networkWatcher(new NetWorkWatcher(this)),
    m_timer(new QTimer(this))
{
    initUI();
}

WeChatAuthDialog::WeChatAuthDialog(int type, QWidget *parent) : QWidget(parent),
    m_networkManager(new QNetworkAccessManager(this)),
    m_loginQR(new QRLabel(this)),
    m_title(new QLabel(this)),
    m_guide(new QLabel(this)),
    m_errorMsg(new QLabel(this)),
    m_networkWatcher(new NetWorkWatcher(this)),
    m_timer(new QTimer(this))
{
    m_authType = type;
    initUI();
}

void WeChatAuthDialog::initUI()
{
    /**
     * ************标题***************
     * **********提示信息**************
     * **********二维码****************
     * **********错误信息***************
     */

    this->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *vbLayout = new QVBoxLayout(this);
    vbLayout->setContentsMargins(0, 0, 0, 0);

    if (m_authType == authType::login)
    {
        m_title->setText(tr("Login by wechat"));
        m_guide->setText(tr("「 Use registered WeChat account to login 」"));
    } else if (m_authType == authType::check)
    {
        m_title->setText(tr("Verification by wechat"));
        m_guide->setText(tr("「 Use bound WeChat account to verification 」"));
    }

    m_errorMsg->setText("");
    m_title->setStyleSheet("font-size:32px;color:#ffffff");
    m_guide->setStyleSheet("font-size:24px;color: rgba(255,255,255,192);");
    m_errorMsg->setStyleSheet("font-size:14px;color:#FD625E;");
    m_guide->setWordWrap(true);
    m_title->adjustSize();
    m_guide->adjustSize();

    QPixmap pixmap(":/image/assets/intel/icon-wechat-noqrcode.png");
    m_loginQR->setPixmap(pixmap.scaled(188, 188, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    m_loginQR->setStyleSheet("QLabel{border-radius: 16px;background:rgb(255,255,255);color:white}");
    //m_loginQR->resize(208,208);

    vbLayout->addWidget(m_title, 0, Qt::AlignCenter);
    vbLayout->addSpacerItem(new QSpacerItem(0,16,QSizePolicy::Fixed,QSizePolicy::Preferred));
    vbLayout->addWidget(m_guide, 0, Qt::AlignCenter);
    vbLayout->addSpacerItem(new QSpacerItem(0,30,QSizePolicy::Fixed,QSizePolicy::Preferred));
    vbLayout->addWidget(m_loginQR, 0, Qt::AlignCenter);
    vbLayout->addWidget(m_errorMsg, 0 ,Qt::AlignCenter);
    vbLayout->addStretch(1);
    this->adjustSize();



//    //监听网络状态变化
//    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
//                                         QString("/org/freedesktop/NetworkManager"),
//                                         QString("org.freedesktop.NetworkManager"),
//                                         QString("StateChanged"), this, SLOT(onNetworkStateChanged(uint)));

    connect(m_networkWatcher, &NetWorkWatcher::NetworkStateChanged, this, &WeChatAuthDialog::onNetworkStateChanged);

    connect(m_timer, &QTimer::timeout, this, [=]{
        m_errorMsg->clear();
    });

}

void WeChatAuthDialog::onNetworkStateChanged(uint state)
{
    if(isHidden())
        return;

    if(NM_STATE_CONNECTED_GLOBAL == state)
        showErrorMessage(" ", true);
    else
        showErrorMessage(tr("Network not connected~"), true);

}

void WeChatAuthDialog::onReset()
{
    if (m_authType == authType::login)
    {
        m_title->setText(tr("Login by wechat"));
        m_guide->setText(tr("「 Use registered WeChat account to login 」"));
    } else if (m_authType == authType::check)
    {
        m_title->setText(tr("Verification by wechat"));
        m_guide->setText(tr("「 Use bound WeChat account to verification 」"));
    }
    m_guide->show();

    QPixmap pixmap(":/image/assets/intel/icon-wechat-noqrcode.png");
    m_loginQR->setPixmap(pixmap.scaled(188, 188, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    m_loginQR->setStyleSheet("QLabel{border-radius: 16px;background:rgb(255,255,255);color:white}");
    //m_loginQR->resize(208,208);
    reloadQR();
}

void WeChatAuthDialog::initConnect()
{
    QObject::connect(m_networkManager, &QNetworkAccessManager::finished,
                     this, &WeChatAuthDialog::replyFinished);
}

void WeChatAuthDialog::getLoginQR()
{
    if(!m_requestQRThread)
    {
        qDebug() << "[WeChatAuthDialog] getLoginQR create new requestQRThread";
        m_requestQRThread = new RequestQRThread(this);
        connect(m_requestQRThread, &RequestQRThread::resultReady, this, &WeChatAuthDialog::onQRUrlReady);
    }

    if(!m_isThreadWorking){
        qDebug() << "[WeChatAuthDialog] getLoginQR requestQRThread is not running";
        m_requestQRThread->start();
        m_isThreadWorking = true;
    } else {
        qDebug() << "[WeChatAuthDialog] getLoginQR requestQRThread is running";
    }
}

void WeChatAuthDialog::replyFinished(QNetworkReply *reply)
{
    if(!reply->isFinished())
    {
        //超时
        reply->abort();
        qWarning() << "error: [WeChatAuthDialog][replyFinished]: network timeout";
        showErrorMessage(tr("Network not connected~"));
        reloadQR();
        return;
    }

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();

        QSize size(188, 188);
        QBitmap mask(size);
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
        painter.setBrush(QColor(0, 0, 0));
        painter.drawRoundedRect(0, 0, size.width(), size.height(), 16, 16);

        QPixmap pixmap;
        pixmap.loadFromData(bytes);
        QPixmap pixmap_ = pixmap.scaled(188, 188, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        pixmap_.setMask(mask);
        m_loginQR->setPixmap(pixmap_);
        //m_loginQR->resize(208,208);
    }
    else
    {
        qWarning() << "网络错误" << reply->error();
        reloadQR();
        // 错误处理-显示错误信息，或显示上一次缓存的图片或叉图。
    }
}

void WeChatAuthDialog::qRStatusChangeSolt(QString loginname, QString loginpwd, int curstatus)
{
    qDebug() << "---loginname:" << loginname << "---loginpwd:" << loginpwd << "---curstatus:" << curstatus;
    QRCodeSwepState state = static_cast<QRCodeSwepState>(curstatus);
    switch (state) {
    case QRCodeSwepState::WaitingSwep:
        qDebug() << "info: [WeChatAuthDialog][qRStatusChangeSolt]: waiting user swep qrcode!";
        break;
    case QRCodeSwepState::HaveSwep:
        qDebug() << "info: [WeChatAuthDialog][qRStatusChangeSolt]: user has swep code!";
        m_title->setText(tr("Scan code successfully"));
        m_guide->setText(" ");
        m_errorMsg->setText("");
        break;
    case QRCodeSwepState::CancelSwep:
        qDebug() << "info: [WeChatAuthDialog][qRStatusChangeSolt]: user cancel swep code!";
        onReset();
        m_errorMsg->setText("");
        break;
    case QRCodeSwepState::ConfirmSuccess:
    {
        //Todo  判断是否是该用户
        qDebug() << "info: [WeChatAuthDialog][qRStatusChangeSolt]: user = [" << loginname << "] confirm success!";
        Q_EMIT qRStatusChange(loginname, loginpwd, curstatus);
    }
    case QRCodeSwepState::QRCodeInvalid:
    {
        qDebug() << "info: [WeChatAuthDialog][onQRCodeStateChanged]: qrcode invalid!";
        onReset();
        break;
    }case QRCodeSwepState::QRCodeTimeout:
    {
        qDebug() << "info: [UserSecondQRCodeLogin][onQRCodeState]: qrcode timeout!";
        onReset();
        // 更新错误信息
        showErrorMessage(tr("Timeout!Try again!"));
        break;
    }
    default:
        break;
    }
}


void WeChatAuthDialog::reloadQR()
{
    if(this->parentWidget()->isHidden() || isHidden())
        return;
    qDebug() << "info: [WeChatAuthDialog] [reloadQR] reloadQR";
    getLoginQR();
}

void WeChatAuthDialog::showErrorMessage(QString message, bool isLong)
{
    if (m_timer->isActive())
        m_timer->stop();
    onReset();
    m_errorMsg->setText(message);
    m_errorMsg->adjustSize();
    m_errorMsg->setGeometry((this->width() - m_errorMsg->width())/2,
                            this->height() - m_errorMsg->height(),
                            m_errorMsg->width(),
                            m_errorMsg->height());
//    if(isLong)
//        return;
//    QTimer *timer = new QTimer(this);
//    connect(timer, &QTimer::timeout, this, [=]{
//        m_errorMsg->setText(" ");
//    });
//    timer->start(3000);
    m_timer->setSingleShot(true);
    m_timer->start(10000);
}

void WeChatAuthDialog::onQRUrlReady(int errorCode, const QString url)
{
    m_isThreadWorking = false;
    if (DBusMsgCode(errorCode) == DBusMsgCode::No_Error)
    {
        // 从网络上下载二维码   并加载到qrcode
        QNetworkRequest request;
        request.setUrl(url);
        QNetworkReply *reply = m_networkManager->get(request);
        QTimer::singleShot(2*1000, this, [=](){
            replyFinished(reply);
        });
    } else if(DBusMsgCode(errorCode) == DBusMsgCode::Error_NetWork)
    {
        qWarning() << "[error] [WeChatAuthDialog] DBus(cn.kylinos.SSOBackend.eduplatform) get url network error";
        QTimer::singleShot(2 * 1000, this, [=]{
            if(this->isVisible() && !m_requestQRThread->isRunning())
                reloadQR();
        });
    }
    else {
        qWarning() << "[error] <WeChatAuthDialog> connect form DBus(cn.kylinos.SSOBackend.eduplatform)";
        QTimer::singleShot(2 * 1000, this, [=](){ reloadQR(); });
    }
}

void WeChatAuthDialog::closeEvent(QCloseEvent *event)
{
    if(m_requestQRThread) {
        qDebug() << "[WeChatAuthDialog] requestQRThread delete";
        m_requestQRThread->quit();
        m_requestQRThread->wait();
        m_requestQRThread->deleteLater();
    }
}

void WeChatAuthDialog::showEvent(QShowEvent *event)
{
    m_networkWatcher->checkOnline();

    onReset();
    connect(EduPlatformInterface::getInstance(),&EduPlatformInterface::QRStatusChange,
            this, &WeChatAuthDialog::qRStatusChangeSolt);
}

void WeChatAuthDialog::hideEvent(QHideEvent *event)
{
    disconnect(EduPlatformInterface::getInstance(),&EduPlatformInterface::QRStatusChange,
               this, &WeChatAuthDialog::qRStatusChangeSolt);
}

QRLabel::QRLabel(QWidget *parent) : QLabel(parent)
{
}

void QRLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button()== Qt::LeftButton)
    {
        Q_EMIT clicked();
    }
}

RequestQRThread::RequestQRThread(QObject *parent) : QThread(parent)
{
    //
}

void RequestQRThread::run()
{
    QString url;
    DBusMsgCode ret = EduPlatformInterface::getInstance()->CheckUserByQR(url);
    Q_EMIT resultReady(ret, url.toLocal8Bit());
}
