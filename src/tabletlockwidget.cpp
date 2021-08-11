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
#include <QDateTime>
#include <QTimer>
#include <QDebug>
#include <QMenu>
//#include <QtX11Extras/QX11Info>
//#include <X11/Xlib.h>
//#include <X11/XKBlib.h>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFont>
#include <QPalette>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusReply>

#include "tabletlockwidget.h"
#include "authdialog.h"
#include "virtualkeyboard.h"
#include "users.h"
#include "displaymanager.h"
#include "weathermanager.h"
#include "powermanager.h"
#include "digitalauthdialog.h"
#include "switchbuttongroup.h"
#include "wechatauthdialog.h"
#include "verificationwidget.h"
#include "verticalVerificationwidget.h"
#include "common.h"
#include "eduplatforminterface.h"
#include "accountsinterface.h"
#include "kylinnm.h"

#include <QVBoxLayout>

float tablescale;
TabletLockWidget::TabletLockWidget(QWidget *parent) :
    QWidget(parent),
    displayManager(new DisplayManager(this)),
    m_switchButtonGroup(new SwitchButtonGroup(this)),
    m_digitalAuthDialog(new DigitalAuthDialog(this)),
    m_weChatAuthDialog(new WeChatAuthDialog(this)),
    m_pb_powerManager(new QPushButton(this)),
    m_pb_networkManager(new QPushButton(this)),
    m_vKeyboardPB(new QPushButton(this)),
    m_vKeyboard(new VirtualKeyboard(this)),
    m_cancelOrBack(new QPushButton(this)),
    m_pb_skip(new QPushButton(this)),
    m_verificationWidget(new VerificationWidget(this)),
    m_verticalVerificationWidget(new VerticalVerificationWidget(this)),
    m_authType(0),
    m_eduPlatformInterface(nullptr),
    m_kylinNM(new KylinNM(this))
{
    tablescale = 1.0;
    m_kylinNM->installEventFilter(this);
    m_digitalAuthDialog->installEventFilter(this);
    m_switchButtonGroup->installEventFilter(this);
    m_vKeyboard->installEventFilter(this);
    this->installEventFilter(this);

    powermanager = new PowerManager(this);
    powermanager->hide();

    initUI();
    initConnect();
    onSwitchPage();
}

TabletLockWidget::~TabletLockWidget()
{
}

void TabletLockWidget::closeEvent(QCloseEvent *event)
{
    qDebug() << "TabletLockWidget::closeEvent";
    m_digitalAuthDialog->close();
    m_weChatAuthDialog->close();
    m_switchButtonGroup->close();
    m_verificationWidget->close();
    m_verticalVerificationWidget->close();
    m_kylinNM->close();
    return QWidget::closeEvent(event);
}


bool TabletLockWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_vKeyboard || obj == m_kylinNM)
        return true;

    if (event->type() == QEvent::MouseButtonPress && m_kylinNM->isVisible())
    {
        m_kylinNM->hide();
        if(powermanager->isHidden()) //非电源会话界面则刷新，否则只网络弹窗
            onSwitchPage();
    }

    return QWidget::eventFilter(obj, event);
}

void TabletLockWidget::startAuth()
{
    m_digitalAuthDialog->startAuth();
}

void TabletLockWidget::stopAuth()
{

}

void TabletLockWidget::initUI()
{
    //电源管理
    m_pb_powerManager->setIcon(QIcon(":/image/assets/intel/powerManager.png"));
    m_pb_powerManager->setFixedSize(48,48);
    m_pb_powerManager->setIconSize(QSize(24,24));
    m_pb_powerManager->setFocusPolicy(Qt::NoFocus);
    m_pb_powerManager->setStyleSheet("QPushButton:Hover{border-radius:24px;background-color:rgba(255, 255, 255, 0.15);}"
                                 "QPushButton:Pressed{border-radius:24px;background-color:rgba(255, 255, 255, 0.05);}");
    m_pb_powerManager->installEventFilter(this);

    connect(m_pb_powerManager,&QPushButton::clicked
            ,this,&TabletLockWidget::showPowerManager);

    //网络设置
    updateNetIcon(m_kylinNM->getConnectStatus());
    m_pb_networkManager->setFixedSize(48,48);
    m_pb_networkManager->setIconSize(QSize(24,24));
    m_pb_networkManager->setFocusPolicy(Qt::NoFocus);
    m_pb_networkManager->setStyleSheet("QPushButton:Hover{border-radius:24px;background-color:rgba(255, 255, 255, 0.15);}"
                                       "QPushButton:Pressed{border-radius:24px;background-color:rgba(255, 255, 255, 0.05);}");
    m_pb_networkManager->installEventFilter(this);

    connect(m_pb_networkManager,&QPushButton::clicked
            ,this,&TabletLockWidget::showNetManager);

    //虚拟键盘
    m_vKeyboard->hide();

    connect(m_vKeyboard, &VirtualKeyboard::aboutToClose,
            m_vKeyboard, &VirtualKeyboard::hide);

    m_vKeyboardPB->setIcon(QIcon(":/image/assets/intel/keyboard.png"));
    m_vKeyboardPB->setFixedSize(48,48);
    m_vKeyboardPB->setIconSize(QSize(24,24));
    m_vKeyboardPB->setFocusPolicy(Qt::NoFocus);
    m_vKeyboardPB->installEventFilter(this);
    m_vKeyboardPB->setStyleSheet("QPushButton:Hover{border-radius:24px;background-color:rgba(255, 255, 255, 0.15);}"
                                 "QPushButton:Pressed{border-radius:24px;background-color:rgba(255, 255, 255, 0.05);}");

    connect(m_vKeyboardPB,&QPushButton::clicked
            ,this,&TabletLockWidget::showVirtualKeyboard);

    //取消或返回
    m_cancelOrBack->resize(88, 48);
    m_cancelOrBack->setIcon(QIcon(":/image/assets/intel/cancel.png"));
    m_cancelOrBack->setStyleSheet("QPushButton{font-size:16px;color:#ffffff;}"
                                  "QPushButton:Hover{border-radius:24px;background-color:rgba(255, 255, 255, 0.15);}"
                                  "QPushButton:Pressed{border-radius:24px;background-color:rgba(255, 255, 255, 0.05);}");
    m_cancelOrBack->setText(tr("Cancel"));
    m_cancelOrBack->setIconSize(QSize(24,24));
    m_cancelOrBack->setFocusPolicy(Qt::NoFocus);
    m_cancelOrBack->installEventFilter(this);
    connect(m_digitalAuthDialog, &DigitalAuthDialog::requestPasswordReset, this, [=]{
        m_cancelOrBack->setIcon(QIcon(":/image/assets/intel/back.png"));
        m_cancelOrBack->setText(tr("Back"));
        m_pageType = pageType::check;
        m_pb_skip->hide();
        onSwitchPage();
    });
    connect(m_cancelOrBack, &QPushButton::clicked, this, [=]{
        if(powermanager->isVisible())
        {
            powermanager->hide();
            onSwitchPage();
            //            m_digitalAuthDialog->reset();
            if(m_pageType != pageType::check)
            {
                m_cancelOrBack->setIcon(QIcon(":/image/assets/intel/cancel.png"));
                m_cancelOrBack->setText(tr("Cancel"));
            }
            return;
        }

        if(m_pageType == pageType::pinLogin &&
                m_digitalAuthDialog->getStatus() == DigitalAuthDialog::LoginType::SECONDCONFIRMATION)
        {
            // 二次确认页返回，则显示重置密码页
            m_digitalAuthDialog->showResetPasswordPage();
            return;
        }

        if(m_pageType == pageType::check) {
            m_pageType = pageType::pinLogin;
            m_digitalAuthDialog->reset();

            onSwitchPage();
            m_cancelOrBack->setIcon(QIcon(":/image/assets/intel/cancel.png"));
            m_cancelOrBack->setText(tr("Cancel"));
            return;
        }

        if(m_digitalAuthDialog->getStatus() == 0)
        {
            //Todo 1、锁屏 -》息屏
            m_digitalAuthDialog->reset();
            Q_EMIT blackSaver();
#ifdef USE_INTEL	    
            powermanager->hibernate();
#endif	    
        } else {
            m_digitalAuthDialog->reset();
            m_cancelOrBack->setIcon(QIcon(":/image/assets/intel/cancel.png"));
            m_cancelOrBack->setText(tr("Cancel"));
        }
    });

    // 重置密码时跳过按钮
    m_pb_skip->setText(tr("Skip"));
    m_pb_skip->resize(88, 48);
    m_pb_skip->setStyleSheet("QPushButton{font-size:16px;color:#ffffff;}"
                                  "QPushButton:Hover{border-radius:24px;background-color:rgba(255, 255, 255, 0.15);}"
                                  "QPushButton:Pressed{border-radius:24px;background-color:rgba(255, 255, 255, 0.05);}");
    m_pb_skip->hide();

    //网络模块
    m_kylinNM->hide();
}

/**
 * @brief TabletLockWidget::initConnect
 * 建立信号与槽的绑定
 */
void TabletLockWidget::initConnect()
{
    connect(m_digitalAuthDialog, &DigitalAuthDialog::authenticateCompete,
            this, &TabletLockWidget::closed);
    connect(m_digitalAuthDialog, &DigitalAuthDialog::setPinCode, this, [=](QString pinCode){
        //Todo
        QString currentPinCode;
        AccountsInterface::getInstance()->GetAccountPincode(getenv("USER"), currentPinCode);
        qDebug() << "currentPinCode:" << currentPinCode << "--old pinCode:" << pinCode;
        if(!currentPinCode.isEmpty() && currentPinCode == pinCode)
        {
            //与原密码相同
            m_digitalAuthDialog->showErrorMessage(tr("New password is the same as old"));
            m_digitalAuthDialog->showResetPasswordPage();
            return;
        }

        DBusMsgCode code = AccountsInterface::getInstance()->SetAccountPincode(getenv("USER"), pinCode);
        if (code == DBusMsgCode::No_Error) {
            qDebug() << "set pincode success";
            Q_EMIT closed();
        } else {
            qWarning() << "设置失败" << code;
            m_digitalAuthDialog->showErrorMessage(tr("Reset password error:%1").arg(code));
        }
    });
    connect(m_weChatAuthDialog, &WeChatAuthDialog::authenticateCompete, this, &TabletLockWidget::closed);
    connect(m_weChatAuthDialog, &WeChatAuthDialog::qRStatusChange, this, [=] (QString loginname, QString loginpwd, int curstatus){
        //QString loginname, QString loginpwd, int curstatus
        if(getenv("USER") == loginname && m_pageType == pageType::wecharLogin)
        {
            Q_EMIT closed();
        } else {
            qWarning() << "[error] <TabletLockWidget> Username does not match wechat<" << loginname << ">" << " local<" << getenv("USER")<<">";
            if (!loginname.isEmpty()){
                m_weChatAuthDialog->showErrorMessage(tr("Please scan by correct WeChat"));
                m_weChatAuthDialog->onReset();
            }
        }
    });
    connect(m_switchButtonGroup, &SwitchButtonGroup::onSwitch, this, &TabletLockWidget::switchLoginType);
    connect(m_verificationWidget, &VerificationWidget::pageMessage, this, [=](SwitchPage type, QList<QVariant> args)
    {
        m_pageType = pageType::pinLogin;
        onSwitchPage();
        m_pb_skip->show();
    });
    connect(m_verticalVerificationWidget, &VerticalVerificationWidget::pageMessage, this, [=](SwitchPage type, QList<QVariant> args)
    {
        m_pageType = pageType::pinLogin;
        onSwitchPage();
        m_pb_skip->show();
    });

    connect(powermanager,SIGNAL(lock())
            ,this,SLOT(showPowerManager()));

    if(QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.tablet-mode"))
    {
        QGSettings *tm = new QGSettings("org.ukui.SettingsDaemon.plugins.tablet-mode", "", this);
        //判断是否是平板模式
        if(tm->get("tablet-mode").toBool()){
            connect(m_kylinNM, &KylinNM::onLineEditClicked, this, [=](){
                //点击了输入密码的框框，需要弹出软键盘
                showVirtualKeyboard(true);
                m_kylinNM->move(m_kylinNM->x(), height() - m_vKeyboard->height() - m_kylinNM->height() - 20);
            });
        }

        //监听平板模式变化
        connect(tm, &QGSettings::changed, this, [=](QString key){
            if(key == "tablet-mode" || key == "tabletMode") {
                disconnect(m_kylinNM, &KylinNM::onLineEditClicked, this, nullptr);
                if(tm->get("tablet-mode").toBool()){
                    connect(m_kylinNM, &KylinNM::onLineEditClicked, this, [=](){
                        //点击了输入密码的框框，需要弹出软键盘
                        showVirtualKeyboard(true);
                        m_kylinNM->move(m_kylinNM->x(), height() - m_vKeyboard->height() - m_kylinNM->height());
                    });
                }
            }
        });
    }

    connect(m_kylinNM, &KylinNM::onConnectChanged, this,[=](){
        updateNetIcon(m_kylinNM->getConnectStatus());
    });

    connect(m_vKeyboard, &VirtualKeyboard::aboutToClose, m_vKeyboard, [=](){
        if(m_vKeyboard->isHidden() && m_kylinNM->isVisible()) {
            m_kylinNM->setGeometry(this->width() - m_kylinNM->width() - 20,
                                   this->height() - m_kylinNM->height() - 100,
                                   m_kylinNM->width(),
                                   m_kylinNM->height());
        }
    });

    connect(m_pb_skip, &QPushButton::clicked, this, &TabletLockWidget::closed);
    connect(m_digitalAuthDialog, &DigitalAuthDialog::switchToReset, this, [=](bool isReset){
       if(isReset){
           m_pb_skip->show();
       } else {
           m_pb_skip->hide();
       }
    });
}

void TabletLockWidget::switchLoginType(int position)
{
    if(position == 0)
    {
        //锁屏密码登录
        m_pageType = pageType::pinLogin;
    } else if (position == 1)
    {
        m_pageType = pageType::wecharLogin;
        //微信登录
        m_digitalAuthDialog->reset();
        m_cancelOrBack->setIcon(QIcon(":/image/assets/intel/cancel.png"));
        m_cancelOrBack->setText(tr("Cancel"));
    }

    onSwitchPage();
}

void TabletLockWidget::showPowerManager()
{
    if (powermanager->isVisible())
    {
        powermanager->hide();
        onSwitchPage();
        if(m_pageType != pageType::check)
        {
            m_cancelOrBack->setIcon(QIcon(":/image/assets/intel/cancel.png"));
            m_cancelOrBack->setText(tr("Cancel"));
        }
        Q_EMIT screenSaver();
    } else {
        m_digitalAuthDialog->hide();
        m_weChatAuthDialog->hide();
        m_switchButtonGroup->hide();
        m_verificationWidget->hide();
        m_verticalVerificationWidget->hide();
        m_cancelOrBack->setIcon(QIcon(":/image/assets/intel/back.png"));
        m_cancelOrBack->setText(tr("Back"));
        m_pb_networkManager->hide();
        m_pb_powerManager->hide();
        m_vKeyboardPB->hide();
        m_vKeyboard->hide();
        powermanager->show();
        powermanager->setGeometry((width()-ITEM_WIDTH*4 - ITEM_SPACING*3)/2,
                                  (height()-ITEM_HEIGHT)/2,
                                  (ITEM_WIDTH*4 + ITEM_SPACING*3),ITEM_HEIGHT);
    }
}

void TabletLockWidget::showNetManager()
{
    if(m_kylinNM->isVisible())
    {
        m_kylinNM->hide();
    } else {
        m_digitalAuthDialog->releaseKeyboard();
        m_kylinNM->show();
        m_kylinNM->setGeometry(this->width() - m_kylinNM->width() - 20,
                               this->height() - m_kylinNM->height() - 100,
                               m_kylinNM->width(),
                               m_kylinNM->height());
        m_kylinNM->updateWifiList();
    }
}

/* lockscreen follows cursor */
void TabletLockWidget::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    //重新计算缩放比例
    tablescale = QString::number(size.width() / 1920.0, 'f', 1).toFloat();

    if(tablescale > 1)
        tablescale = 1;

    m_cancelOrBack->setGeometry(32, 40, m_cancelOrBack->width(), m_cancelOrBack->height());
    m_pb_skip->setGeometry(this->width() - m_pb_skip->width() - 32, 40, m_pb_skip->width(), m_pb_skip->height());

    m_digitalAuthDialog->setGeometry((this->width()-m_digitalAuthDialog->width())/2, (this->height()-m_digitalAuthDialog->height())/2 + 40,
                                     m_digitalAuthDialog->width(), m_digitalAuthDialog->height());
    m_weChatAuthDialog->setGeometry((this->width()-m_weChatAuthDialog->width())/2, (this->height()-m_digitalAuthDialog->height())/2 + 40,
                                    m_weChatAuthDialog->width(), m_weChatAuthDialog->height());

    m_switchButtonGroup->setGeometry((this->width()-m_switchButtonGroup->width())/2, this->height()-80,
                                     m_switchButtonGroup->width(), m_switchButtonGroup->height());

    if(isOrientation())
      m_verticalVerificationWidget->setGeometry((this->width()-m_verticalVerificationWidget->width())/2, (this->height()-m_verticalVerificationWidget->height())/2,
                                          m_verticalVerificationWidget->width(), m_verticalVerificationWidget->height());
    else
       m_verificationWidget->setGeometry((this->width()-m_verificationWidget->width())/2, (this->height()-m_verificationWidget->height())/2 - 65,
                                          m_verificationWidget->width(), m_verificationWidget->height());
    if(m_verificationWidget->isVisible() || m_verticalVerificationWidget->isVisible())
        showVerificationPage();
    //    XSetInputFocus(QX11Info::display(),this->winId(),RevertToParent,CurrentTime);
    //右下角按钮,x,y的初始值代表距离右下角的距离。
    int x=19,y=76;
    x = x + m_pb_powerManager->width();
    m_pb_powerManager->setGeometry(this->width() - x,this->height() - y, m_pb_powerManager->width(), m_pb_powerManager->height());
    m_pb_networkManager->setGeometry(this->width() - x - 64 * tablescale, this->height() - y,
                                     m_pb_networkManager->width(),m_pb_networkManager->height());
    m_vKeyboardPB->setGeometry(this->width() - x - 128 * tablescale, this->height() - y,
                               m_vKeyboardPB->width(), m_vKeyboardPB->height());
    m_kylinNM->setGeometry(this->width() - m_kylinNM->width() - 20,
                           this->height() - m_kylinNM->height() - 100,
                           m_kylinNM->width(),
                           m_kylinNM->height());
    //更新软键盘位置
    setVirkeyboardPos();
    if(powermanager && powermanager->isVisible())
    {
        powermanager->setGeometry((width()-ITEM_WIDTH*4 - ITEM_SPACING*3)/2,
                                  (height()-ITEM_HEIGHT)/2,
                                  (ITEM_WIDTH*4 + ITEM_SPACING*3),ITEM_HEIGHT);
    }
}

//重写重绘事件，比如动态更新文本位置等
void TabletLockWidget::paintEvent(QPaintEvent *event)
{
}

void TabletLockWidget::onSwitchPage()
{
    bool isOnlyWeChatLogin = false;
    EduPlatformInterface::getInstance()->CheckPincodeSet(getenv("USER"), isOnlyWeChatLogin);
    if(isOnlyWeChatLogin && m_pageType == pageType::pinLogin)
    {
        m_pageType = pageType::wecharLogin;
    }

    if(!m_pb_networkManager->isVisible())
        m_pb_networkManager->show();

    if(!m_pb_powerManager->isVisible())
        m_pb_powerManager->show();

    switch (m_pageType) {
    case pageType::pinLogin:
        m_digitalAuthDialog->show();
        m_digitalAuthDialog->grabKeyboard();
        m_switchButtonGroup->show();
        m_weChatAuthDialog->hide();
        m_verificationWidget->hide();
        m_verticalVerificationWidget->hide();
        m_vKeyboardPB->hide();
        m_vKeyboard->hide();
        break;
    case pageType::wecharLogin:
        m_digitalAuthDialog->hide();
        m_digitalAuthDialog->releaseKeyboard();
        m_weChatAuthDialog->show();
        if(isOnlyWeChatLogin)
        {
            m_switchButtonGroup->hide();
        } else {
            m_switchButtonGroup->show();
        }
        m_verificationWidget->hide();
        m_verticalVerificationWidget->hide();
        m_vKeyboardPB->hide();
        m_vKeyboard->hide();
        break;
    case pageType::check:
        m_digitalAuthDialog->hide();
        m_digitalAuthDialog->releaseKeyboard();
        m_weChatAuthDialog->hide();
        m_switchButtonGroup->hide();
        showVerificationPage();
        m_vKeyboardPB->show();
        break;
    default:
        break;
    }
}

QWidget* TabletLockWidget::getCurrentWidget()
{
    switch (m_pageType) {
    case pageType::pinLogin:
        return m_digitalAuthDialog;
    case pageType::wecharLogin:
        return m_weChatAuthDialog;
    case pageType::check:
        return m_verificationWidget;
    default:
        return m_digitalAuthDialog;
    }
}

void TabletLockWidget::showVirtualKeyboard(bool isShow)
{
    m_vKeyboard->setVisible(isShow || m_vKeyboard->isHidden());
    setVirkeyboardPos();
}

void TabletLockWidget::setVirkeyboardPos()
{
    if(m_vKeyboard)
    {
        m_vKeyboard->setGeometry(0 + 20,
                                 height() - height()/3 + 10,
                                 width() - 40 , height()/3 - 20);
        m_vKeyboard->raise();
        m_vKeyboard->repaint();
    }
}

bool TabletLockWidget::isOrientation()
{
    QRect wRect = QApplication::primaryScreen()->geometry();
    if(wRect.height() > wRect.width())
        return true;
    return false;
}

void TabletLockWidget::showVerificationPage()
{
    if(isOrientation())
    {
        m_verificationWidget->hide();
        m_verticalVerificationWidget->show();
    }
    else
    {
        m_verificationWidget->show();
        m_verticalVerificationWidget->hide();
    }
}

//void TabletLockWidget::RecieveKey(int key)
//{
//    if(m_pageType == pageType::pinLogin ||m_pageType == pageType::check)
//        m_digitalAuthDialog->RecieveKey(key);
//}

void TabletLockWidget::updateNetIcon(int status)
{
    switch(status) {
    case 0:
        //有线
        m_pb_networkManager->setIcon(QIcon(":/image/assets/intel/icon-wired.png"));
        break;
    case 1:
        //无线
        m_pb_networkManager->setIcon(QIcon(":/image/assets/intel/icon-wifi.png"));
        break;
    case 2:
        //有线+无线
        m_pb_networkManager->setIcon(QIcon(":/image/assets/intel/icon-wired.png"));
        break;
    default:
        //无连接
        m_pb_networkManager->setIcon(QIcon(":/image/assets/intel/icon-no-signal.png"));
    }
}

/**
 * @Destoryed
 * @brief TabletLockWidget::getEduPlatformInterface
 * @return
 */
EduPlatformInterface* TabletLockWidget::getEduPlatformInterface()
{
    if(m_eduPlatformInterface == nullptr)
    {
        m_eduPlatformInterface = new EduPlatformInterface("cn.kylinos.SSOBackend",
                                                          "/cn/kylinos/SSOBackend",
                                                          QDBusConnection::systemBus(),
                                                          this);
    }

    return m_eduPlatformInterface;
}
