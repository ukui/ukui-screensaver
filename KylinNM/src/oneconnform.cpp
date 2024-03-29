/*
 * Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "oneconnform.h"
#include "ui_oneconnform.h"
#include "kylinnm.h"
#include "kylin-dbus-interface.h"
#include "kylin-network-interface.h"
#include "wireless-security/dlgconnhidwifi.h"
#include "utils.h"
#include <QCheckBox>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QDBusReply>
#include <QDBusObjectPath>

extern int currentActWifiSignalLv;

OneConnForm::OneConnForm(QWidget *parent, KylinNM *mainWindow, ConfForm *confForm, KSimpleNM *ksnm) :
    QWidget(parent),
    ui(new Ui::OneConnForm)
{
    ui->setupUi(this);

    ui->btnConnSub->setText(tr("Connect"));//"设置"
    ui->btnConn->setText(tr("Connect"));//"连接"
    ui->btnConnPWD->setText(tr("Connect"));//"连接"
    ui->btnDisConn->setText(tr("Disconnect"));//"断开连接"
    ui->btnHideConn->setText(tr("Connect"));//"连接"
    ui->lePassword->setPlaceholderText(tr("Input Password..."));//"输入密码..."
    ui->lePassword->setContextMenuPolicy(Qt::NoContextMenu);
    ui->lePassword->setTextMargins(0,0,20,0);

    ui->lbConned->setAlignment(Qt::AlignLeft);
    ui->lePassword->setEchoMode(QLineEdit::Normal);
    ui->btnConnPWD->setEnabled(false);

    leQssLow = "QLineEdit{border:none;background:transparent;font-size:14px;color:rgba(255,255,255,0.57);font-family:Noto Sans CJK SC;}";
    leQssHigh = "QLineEdit{border:none;background:transparent;font-size:14px;color:rgba(255,255,255,0.91);font-family:Noto Sans CJK SC;}";

    ui->leInfo_1->setStyleSheet(leQssLow);
    ui->leInfo_2->setStyleSheet(leQssLow);
    ui->leInfo_3->setStyleSheet(leQssLow);
    ui->leInfo_1->setEnabled(false);
    ui->leInfo_2->setEnabled(false);
    ui->leInfo_3->setEnabled(false);
    ui->btnInfo->setStyleSheet("QPushButton{border:none;background:transparent;}");
    ui->wbg->setStyleSheet("#wbg{border-radius:8px;background-color:rgba(255,255,255,0);}");
    ui->wbg_2->setStyleSheet("#wbg_2{border-radius:8px;background-color:rgba(38, 38, 38, 0.1)}");
    ui->wbg_3->setStyleSheet("#wbg_3{border-radius:4px;background-color:rgba(255,255,255,0.1);}");
    ui->lbName->setStyleSheet("QLabel{font-size:14px;color:rgba(38,38,38,1);}");
    ui->lbConned->setStyleSheet("QLabel{font-size:14px;color:rgba(255,255,255,0.57);}");
    ui->lePassword->setStyleSheet("QLineEdit{border:2px solid rgba(47, 179, 232, 1);border-radius:4px;"
                                  "background:rgba(255,255,255,1);color:rgba(38, 38, 38, 1);font-size:14px;}");
    ui->checkBoxPwd->setStyleSheet("QCheckBox::indicator {width: 18px; height: 9px;}"
                                   "QCheckBox::indicator:checked {image: url(:/res/h/show-pwd.png);}"
                                   "QCheckBox::indicator:unchecked {image: url(:/res/h/hide-pwd.png);}");
    ui->btnConnSub->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(61,107,229,1);color:white;font-size:14px;}"
                               "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(107,142,235,1);}"
                               "QPushButton:Pressed{border-radius:4px;background-color:rgba(50,87,202,1);}");
    ui->btnConn->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(61,107,229,1);color:white;font-size:14px;}"
                               "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(107,142,235,1);}"
                               "QPushButton:Pressed{border-radius:4px;background-color:rgba(50,87,202,1);}");
    ui->btnConnPWD->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(47, 179, 232, 1);color:rgba(255, 255, 255, 1);font-size:14px;}"
                               "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(70, 193, 243, 1);}"
                               "QPushButton:Pressed{border-radius:4px;background-color:rgba(27, 160, 213, 1);}"
                                  "QPushButton:Disabled{border-radius:4px;background-color:rgba(168, 168, 168, 1)}");
    ui->btnDisConn->setStyleSheet("#btnDisConn{font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;"
                                  "font-weight: 400;color: rgba(38,38,38,0.75);line-height: 20px;font-size:14px;"
                                  "border:0px;background-color:rgba(38,38,38,0.1);border-radius:8px;}"
                                  "#btnDisConn:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:8px;background-color:rgba(38,38,38,0.2);}"
                                  "#btnDisConn:Pressed{border-radius:8px;background-color:rgba(38,38,38,0.06);}");//断开链接按钮
    ui->btnInfo_2->setStyleSheet("border:0px;border-radius:8px;border-image:url(:/res/l/pb-network-info.png);");
    ui->btnHideConn->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(61,107,229,1);color:white;font-size:14px;}"
                                   "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(107,142,235,1);}"
                                   "QPushButton:Pressed{border-radius:4px;background-color:rgba(50,87,202,1);}");
    ui->lbWaiting->setStyleSheet("QLabel{border:0px;border-radius:10px;background-color:rgba(61,107,229,1);}");
    ui->lbWaitingIcon->setStyleSheet("QLabel{border:0px;background-color:transparent;}");
    ui->autoConn->setStyleSheet("QCheckBox::indicator {width: 16px; height: 16px;}"
                                   "QCheckBox::indicator:checked {image: url(:/res/g/checkbox-checked.svg);}"
                                   "QCheckBox::indicator:unchecked {image: url(:/res/g/checkbox-unchecked.svg);}");

    ui->btnInfo->setCursor(QCursor(Qt::PointingHandCursor));
    ui->btnInfo->setFocusPolicy(Qt::NoFocus);
    ui->checkBoxPwd->setFocusPolicy(Qt::NoFocus);
    ui->btnConnSub->setFocusPolicy(Qt::NoFocus);
    ui->btnConn->setFocusPolicy(Qt::NoFocus);
    ui->btnConnPWD->setFocusPolicy(Qt::NoFocus);
    ui->btnDisConn->setFocusPolicy(Qt::NoFocus);
    ui->btnHideConn->setFocusPolicy(Qt::NoFocus);

    ui->wbg->show();
    ui->wbg_2->hide();
    ui->wbg_3->hide();
    ui->lbName->show();
    ui->leInfo_1->hide();
    ui->leInfo_2->hide();
    ui->leInfo_3->hide();
    ui->lePassword->hide();
    ui->checkBoxPwd->hide();
    ui->checkBoxPwd->setChecked(false);
    ui->lePassword ->setEchoMode(QLineEdit::Password);
    ui->btnConnSub->hide();
    ui->btnConn->hide();
    ui->btnDisConn->hide();
    ui->btnConnPWD->hide();
    ui->btnHideConn->hide();
    ui->line->show();
    ui->autoConn->hide();
    ui->lbWaiting->hide();
    ui->lbWaitingIcon->hide();
    ui->btnInfo->hide();

    this->mw = mainWindow;
    this->cf = confForm;
    this->ks = ksnm;

    this->isSelected = false;
    this->isActive = false;

    this->setAttribute(Qt::WA_Hover,true);
    this->installEventFilter(this);
    ui->lePassword->setAttribute(Qt::WA_Hover,true);
    ui->lePassword->installEventFilter(this);
    ui->btnInfo->setAttribute(Qt::WA_Hover,true);
    ui->btnInfo->installEventFilter(this);

    //m_notify = new NotifySend();

    connect(ui->lePassword, SIGNAL(returnPressed()), this, SLOT(on_btnConnPWD_clicked()));
    ui->btnConn->setShortcut(Qt::Key_Return);//将字母区回车键与登录按钮绑定在一起

    this->waitTimer = new QTimer(this);
    connect(waitTimer, SIGNAL(timeout()), this, SLOT(waitAnimStep()));

    connect(mw, SIGNAL(waitWifiStop()), this, SLOT(stopWaiting()));
    connect(ui->btnInfo_2,&QPushButton::clicked,this,&OneConnForm::on_btnInfo_2_clicked);
    ui->btnInfo_2->hide();
   // connect(ui->autoConn,&QCheckBox::toggled,this,&OneConnForm::on_autoConn_toggled);

    ui->autoConn->setText(tr("Automatically join the network"));
//    ui->autoConn->setText(tr("自动连接"));

    ui->lbWaitingIcon->move(316+194*mw->isTabletStyle,20);// YYF
    tabletStyle();//平板模式PC桌面样式
}

void OneConnForm::tabletStyle()//平板桌面模式特有设置
{
    this->setFixedWidth(414+194*mw->isTabletStyle);
    ui->wbg->setFixedWidth(414+194*mw->isTabletStyle);
    ui->wbg_2->setFixedWidth(414+194*mw->isTabletStyle);
    ui->wbg_3->setFixedWidth(414+194*mw->isTabletStyle);
    ui->lbName->setFixedWidth(190+194*mw->isTabletStyle);
    ui->btnInfo->setFixedWidth(230+194*mw->isTabletStyle);
    ui->leInfo_1->setFixedWidth(230+194*mw->isTabletStyle);
    ui->leInfo_2->setFixedWidth(230+194*mw->isTabletStyle);
    ui->leInfo_3->setFixedWidth(230+194*mw->isTabletStyle);
    ui->lePassword->setFixedWidth(254+172*mw->isTabletStyle);
    ui->checkBoxPwd->move(286+172*mw->isTabletStyle,75);
    ui->btnConnPWD->move(318+172*mw->isTabletStyle,56);
    ui->btnDisConn->move(316+194*mw->isTabletStyle,8 + 18*mw->isTabletStyle);
    ui->btnInfo_2->move(364+194*mw->isTabletStyle,18);
    ui->lbName->move(63,18);
}

OneConnForm::~OneConnForm()
{
    delete ui;
}

void OneConnForm::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
   // emit selectedOneWifiForm(wifiName, H_WIFI_ITEM_BIG_EXTEND);
    ui->wbg_2->setStyleSheet("#wbg_2{border-radius:8px;background-color:rgba(38,38,38,0.1);}");
}

void OneConnForm::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton ||  !rect().contains(event->pos())) {
        event->ignore();
        return;
    }
    if(ui->lePassword->isVisible()){
        Q_EMIT selectedOneWifiForm(wifiName, H_WIFI_ITEM_BIG_EXTEND);
    }else{
        if (!this->isTopItem) {
            if (!is_connecting) {
                toConnectWirelessNetwork();
            }
        }
    }
    ui->wbg_2->setStyleSheet("#wbg_2{border-radius:8px;background-color:rgba(255,255,255,0);}");
}

//事件过滤器
bool OneConnForm::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->btnInfo) {
        if (event->type() == QEvent::HoverEnter) {
            ui->leInfo_1->setStyleSheet(leQssHigh);
            ui->leInfo_2->setStyleSheet(leQssHigh);
            ui->leInfo_3->setStyleSheet(leQssHigh);
            return true;
        } else if(event->type() == QEvent::HoverLeave) {
            ui->leInfo_1->setStyleSheet(leQssLow);
            ui->leInfo_2->setStyleSheet(leQssLow);
            ui->leInfo_3->setStyleSheet(leQssLow);
            return true;
        }
    } else if (obj == this) {
        if(event->type() == QEvent::HoverEnter) {
            if (!this->isTopItem) {
                if (!this->isSelected) {
                    //ui->btnConn->show();
                    ui->wbg->setStyleSheet("#wbg{border-radius:8px;background-color:rgba(38,38,38,0.05);}");
                    ui->wbg->show();
                }
            }
            return true;
        } else if (event->type() == QEvent::HoverLeave) {
            ui->btnConn->hide();
            ui->wbg->setStyleSheet("#wbg{border-radius:8px;background-color:rgba(255,255,255,0);}");
            ui->wbg->hide();
            return true;
        }
    }

    if (obj == ui->lePassword) {
        if (event->type() == QEvent::MouseButtonPress) {
            Q_EMIT onLineEditClicked();
        }
    }

    return QWidget::eventFilter(obj,event);
}

void OneConnForm::on_autoConn_toggled(bool val)
{
//    kylin_network_set_autoconnect(wifiName.toLatin1().data(),val);
}

// 是否当前连接的网络，字体设置不同
void OneConnForm::setAct(bool isAct)
{
    if (isAct) {
        ui->lbConned->hide();
    } else {
        ui->lbConned->hide();
    }
    ui->lbName->move(63,18);
    isActive = isAct;
}

//点击窗口最上面的item时
void OneConnForm::setTopItem(bool isSelected)
{
    if (isSelected) {
        resize(W_ITEM+194*mw->isTabletStyle, H_ITEM_BIG);
        ui->wbg_3->show();
        ui->leInfo_1->show();
        ui->leInfo_2->show();
        ui->leInfo_3->show();

        this->isSelected = true;
    } else {
        resize(W_ITEM+194*mw->isTabletStyle, H_ITEM);
        ui->lePassword->setText("");
        ui->wbg_3->hide();
        ui->leInfo_1->hide();
        ui->leInfo_2->hide();
        ui->leInfo_3->hide();


        this->isSelected = false;
    }

    ui->wbg->hide();
    ui->wbg_2->hide();
    ui->lbSignal->show();
    ui->lePassword->hide();
    ui->checkBoxPwd->hide();
    ui->line->hide();
    ui->btnConn->hide();
    ui->btnConnPWD->hide();
    ui->btnHideConn->hide();
    ui->btnInfo->show();

    if (isConnected) {
        if (this->isWaiting) {
            ui->lbName->setStyleSheet("QLabel{font-size:14px;color:rgba(38,38,38,0.45);}");
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/pb-top-wifi-offline.png);}");
            ui->btnDisConn->hide();
//            ui->btnInfo_2->hide();
        } else {
            ui->lbName->setStyleSheet("QLabel{font-size:14px;color:rgba(47, 179, 232, 1);}");
            setSignalOn(signalStrong,signalHasPw);
            ui->btnDisConn->show();
//            ui->btnInfo_2->show();
        }
        ui->btnInfo_2->hide();
    } else {
        ui->lbName->setStyleSheet("QLabel{font-size:14px;color:rgba(38,38,38,0.45);}");
        ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/pb-top-wifi-offline.png);}");
        ui->btnDisConn->hide();
        ui->btnInfo_2->hide();
    }

    this->isTopItem = true;
}
// 点击窗口下面的item时
void OneConnForm::setSelected(bool isSelected, bool isCurrName)
{
    if (isSelected) {
        resize(W_ITEM+194*mw->isTabletStyle, H_ITEM_BIG);
        ui->line->move(X_LINE_BIG_EXTEND, Y_LINE_BIG_EXTEND);
        ui->wbg->hide();
        ui->wbg_2->hide();
        ui->wbg_3->show();
        ui->leInfo_1->show();
        ui->leInfo_2->show();
        ui->leInfo_3->show();
        ui->btnConn->hide();
        ui->btnConnSub->show();

        this->isSelected = true;
    } else {
        resize(W_ITEM+194*mw->isTabletStyle, H_ITEM);
        ui->lePassword->setStyleSheet("QLineEdit{border:2px solid rgba(47, 179, 232, 1);border-radius:4px;"
                                      "background:rgba(255,255,255,1);color:rgba(38, 38, 38, 1);font-size:14px;}");

        ui->line->move(X_LINE, Y_LINE);
        ui->wbg->show();
        ui->wbg_2->hide();
        ui->wbg_3->hide();
        ui->leInfo_1->hide();
        ui->leInfo_2->hide();
        ui->leInfo_3->hide();

        if (isCurrName) {
            //ui->btnConn->show();
        } else {
            ui->btnConn->hide();
        }
        ui->btnConnSub->hide();

        this->isSelected = false;
    }

    ui->lePassword->hide();
    ui->checkBoxPwd->hide();
    ui->btnConnPWD->hide();
    ui->lbSignal->show();
    ui->btnDisConn->hide();
    ui->btnHideConn->hide();
    ui->btnInfo->hide();

    this->isTopItem = false;
}
// 点击连接隐藏wifi的item时
void OneConnForm::setHideItem(bool isHideItem, bool isShowHideBtn)
{
    if (isHideItem) {
        ui->lbName->move(14, 30);
        ui->wbg->hide();
        ui->btnConn->hide();
    } else {
        ui->lbName->move(62, 18);
        ui->wbg->show();
        //ui->btnConn->show();
    }

    if (isShowHideBtn) {
        ui->btnHideConn->show();
    } else{
        ui->btnHideConn->hide();
    }
}

void OneConnForm::setConnedString(bool showLable, QString str, QString str1)
{
    if (!showLable) {
        ui->lbConned->setText(str1);
        ui->lbConned->hide();
    } else {
        ui->lbConned->setText(str);
    }
    ui->lbName->move(63, 18);
}

void OneConnForm::setName(QString name)
{
    ui->lbName->setText(name);
    wifiName = name;
}
void OneConnForm::setSpecialName(QString name)
{
    ui->lbName->setText(tr("Connect to Hidden Wi-Fi Network")); //连接到隐藏的 Wi-Fi 网络
    wifiName = name;
}

QString OneConnForm::getName()
{
    return ui->lbName->text();
}

void OneConnForm::setRate(QString rate)
{
//    QString txt(tr("Rate"));//"速率"
//    this->setToolTip("<span style=\"font-size:14px;border:none;background-color:#3593b5;color:white;\">&nbsp; " + txt + ": " + rate + " &nbsp;</span>");
//    this->setToolTip(txt + ":" + rate);
}

void OneConnForm::setLine(bool isShow)
{
    if (isShow) {
        ui->line->show();
    } else {
        ui->line->hide();
    }
}

void OneConnForm::setSignalOn(QString lv, QString secu)
{

    int signal = lv.toInt();
    if (secu == "--" || secu == "") {
        hasPwd = false;
    } else {
        hasPwd = true;
    }

    if (signal > 75) {
        if (hasPwd) {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-full-pwd.png);}");
        } else {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-full.png);}");
        }
        signalLv = 1;
    }
    if (signal > 55 && signal <= 75) {
        if (hasPwd) {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-high-pwd.png);}");
        } else {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-high.png);}");
        }
        signalLv = 2;
    }
    if (signal > 35 && signal <= 55) {
        if (hasPwd) {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-medium-pwd.png);}");
        } else {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-medium.png);}");
        }
        signalLv = 3;
    }
    if (signal > 15 && signal <= 35) {
        if (hasPwd) {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-low-pwd.png);}");
        } else {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-low.png);}");
        }
        signalLv = 4;
    }
    if (signal <= 15) {
        if (hasPwd) {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-none-pwd.png);}");
        } else {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-none.png);}");
        }
        signalLv = 4;
    }
}

void OneConnForm::setSignal(QString lv, QString secu)
{
    signalStrong=lv;
    signalHasPw=secu;

    int signal = lv.toInt();
    if (secu == "--" || secu == "") {
        hasPwd = false;
    } else {
        hasPwd = true;
    }

    if (signal > 75) {
        if (hasPwd) {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-full-pwd-off.png);}");
        } else {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-full-off.png);}");
        }
        signalLv = 1;
    }
    if (signal > 55 && signal <= 75) {
        if (hasPwd) {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-high-pwd-off.png);}");
        } else {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-high-off.png);}");
        }
        signalLv = 2;
    }
    if (signal > 35 && signal <= 55) {
        if (hasPwd) {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-medium-pwd-off.png);}");
        } else {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-medium-off.png);}");
        }
        signalLv = 3;
    }
    if (signal > 15 && signal <= 35) {
        if (hasPwd) {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-low-pwd-off.png);}");
        } else {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-low-off.png);}");
        }
        signalLv = 4;
    }
    if (signal <= 15) {
        if (hasPwd) {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-none-pwd-off.png);}");
        } else {
            ui->lbSignal->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/w/wifi-none-off.png);}");
        }
        signalLv = 4;
    }
}

void OneConnForm::setWifiInfo(QString str1, QString str2, QString str3)
{
    if (str1 == "--" || str1 == ""){ str1 = tr("None"); };

    QString strSecurity = QString(tr("WiFi Security："));
    QString strSignal = QString(tr("Signal："));
    QString strMAC = QString(tr("MAC："));

    ui->leInfo_1->setText(strSecurity + str1);
    ui->leInfo_2->setText(strSignal + str2);
    ui->leInfo_3->setText(strMAC + str3);
}

void OneConnForm::slotConnWifi()
{
    //mw->startLoading();
    this->startWaiting(true);
    emit sigConnWifi(ui->lbName->text());
}
void OneConnForm::slotConnWifiPWD()
{
    //mw->startLoading();
    this->startWaiting(true);
    emit sigConnWifiPWD(ui->lbName->text(), ui->lePassword->text());
}

//点击后断开wifi网络
void OneConnForm::on_btnDisConn_clicked()
{
    syslog(LOG_DEBUG, "DisConnect button about wifi net is clicked, current wifi name is %s .", ui->lbName->text().toUtf8().data());
    qDebug()<<"DisConnect button about wifi net is clicked, current wifi name is "<<ui->lbName->text();

    this->startWaiting(false);

    mw->is_stop_check_net_state = 1;
    mw->on_btnHotspotState();
    QString name = ui->lbName->text();
    kylin_network_set_con_down(name.replace("\"","\\\"").toUtf8().data());
    disconnect(this, SIGNAL(selectedOneWifiForm(QString,int)), mw, SLOT(oneWifiFormSelected(QString,int)));
    emit disconnActiveWifi();
}

//点击列表item扩展时会出现该按钮 用于连接网络
void OneConnForm::on_btnConnSub_clicked()
{
    syslog(LOG_DEBUG, "A button named on_btnConnSub about wifi net is clicked.");
    qDebug()<<"A button named on_btnConnSub about wifi net is clicked.";
    toConnectWirelessNetwork();
}

//无需密码的wifi连接
void OneConnForm::on_btnConn_clicked()
{
//    syslog(LOG_DEBUG, "A button named btnConn about wifi net is clicked.");
//    qDebug()<<"A button named btnConn about wifi net is clicked.";
//    toConnectWirelessNetwork();
    on_btnInfo_clicked();
}

void OneConnForm::toConnectWirelessNetwork()
{
    if (ui->lbConned->text() == "--" || ui->lbConned->text() == " ") {
        if (!isWifiConfExist(ui->lbName->text())) {
            //没有配置文件，使用有密码的wifi连接
            on_btnConnPWD_clicked();
            return;
        }
    }

    mw->is_stop_check_net_state = 1;
    QThread *t = new QThread();
    BackThread *bt = new BackThread();
    bt->moveToThread(t);
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
    connect(t, SIGNAL(started()), this, SLOT(slotConnWifi()));
    connect(this, SIGNAL(sigConnWifi(QString)), bt, SLOT(execConnWifi(QString)));
    connect(bt, &BackThread::connDone, this,[=](int t) {
        mw->connWifiDone(t);
    });
    connect(bt, SIGNAL(connDone(int)), this, SLOT(slotConnWifiResult(int)));
    connect(bt, SIGNAL(btFinish()), t, SLOT(quit()));
    t->start();
    is_connecting = true;
}

//需要密码的wifi连接
void OneConnForm::on_btnConnPWD_clicked()
{
    syslog(LOG_DEBUG, "A button named btnConnPWD about wifi net is clicked.");
    qDebug()<<"A button named btnConnPWD about wifi net is clicked.";

    ui->btnConnPWD->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(47, 179, 232, 1);color:white;font-size:14px;}"
                               "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(70, 193, 243, 1);}"
                               "QPushButton:Pressed{border-radius:4px;background-color:rgba(27,160,213,1);}"
                                  "QPushButton:Disabled{border-radius:4px;background-color:#BEBEBE;}");
    ui->btnConnPWD->setEnabled(false);

    mw->is_stop_check_net_state = 1;
    QThread *t = new QThread();
    BackThread *bt = new BackThread();
    bt->moveToThread(t);
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
    connect(t, SIGNAL(started()), this, SLOT(slotConnWifiPWD()));
    connect(this, SIGNAL(sigConnWifiPWD(QString, QString)), bt, SLOT(execConnWifiPWD(QString, QString)));
    connect(bt, SIGNAL(connDone(int)), mw, SLOT(connWifiDone(int)));
    connect(bt, SIGNAL(connDone(int)), this, SLOT(slotConnWifiResult(int)));
    connect(bt, SIGNAL(btFinish()), t, SLOT(quit()));
    ui->lePassword->setEnabled(false);
    t->start();
}

//点击后弹出连接隐藏wifi网络窗口
void OneConnForm::on_btnHideConn_clicked()
{
//    QApplication::setQuitOnLastWindowClosed(false);
    DlgConnHidWifi *connHidWifi = new DlgConnHidWifi(0, mw, this->parentWidget());
    connect(connHidWifi, SIGNAL(reSetWifiList() ), mw, SLOT(on_btnWifiList_clicked()) );
    connHidWifi->show();
}

bool OneConnForm::isWifiConfExist(QString netName)
{
    //dbusWifiMac = ""; //这个函数之前是用来获取已经连接的wifi的MAC地址

    QDBusInterface m_interface("org.freedesktop.NetworkManager",
                                      "/org/freedesktop/NetworkManager/Settings",
                                      "org.freedesktop.NetworkManager.Settings",
                                      QDBusConnection::systemBus() );
    QDBusReply<QList<QDBusObjectPath>> m_reply = m_interface.call("ListConnections");

    QList<QDBusObjectPath> m_objNets = m_reply.value();
    foreach (QDBusObjectPath objNet, m_objNets){
        QDBusInterface m_interface("org.freedesktop.NetworkManager",
                                  objNet.path(),
                                  "org.freedesktop.NetworkManager.Settings.Connection",
                                  QDBusConnection::systemBus());
        QDBusMessage reply = m_interface.call("GetSettings");
        const QDBusArgument &dbusArg = reply.arguments().at( 0 ).value<QDBusArgument>();
        QMap<QString,QMap<QString,QVariant>> map;
        dbusArg >> map;

        for(QString key : map.keys() ){
            QMap<QString,QVariant> innerMap = map.value(key);
            if (key == "connection") {
                for (QString inner_key : innerMap.keys()) {
                    if (inner_key == "id"){
                        if (netName == innerMap.value(inner_key).toString()) {
                            return true;
                        }
                    }
                }
            }
        }

    } // end foreach (QDBusObjectPath objNet, m_objNets)

    return false;
}

//设置密码隐藏或可见
void OneConnForm::on_checkBoxPwd_stateChanged(int arg1)
{
    if (arg1 == 0) {
        ui->lePassword ->setEchoMode(QLineEdit::Password);
    } else {
        ui->lePassword->setEchoMode(QLineEdit::Normal);
    }
}

void OneConnForm::on_lePassword_textEdited(const QString &arg1)
{
    ui->lePassword->setStyleSheet("QLineEdit{border:2px solid rgba(47, 179, 232, 1);border-radius:4px;"
                                  "background:rgba(255,255,255,1);color:rgba(38, 38, 38, 1);font-size:14px;}");

    if (ui->lePassword->text().size() < 8){
        ui->btnConnPWD->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(47, 179, 232, 1);color:white;font-size:14px;}"
                                   "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(70, 193, 243, 1);}"
                                   "QPushButton:Pressed{border-radius:4px;background-color:rgba(27,160,213,1);}"
                                      "QPushButton:Disabled{border-radius:4px;background-color:rgba(168, 168, 168, 1)}");
        ui->btnConnPWD->setEnabled(false);//连接时设置为不可点击
        if (ui->lePassword->text().size() == 0){
            ui->lePassword->setStyleSheet("QLineEdit{border:2px solid rgba(47, 179, 232, 1);border-radius:4px;"
                                          "background:rgba(255,255,255,1);color:red;font-size:14px;}");
        }
    } else {
        ui->btnConnPWD->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(47, 179, 232, 1);color:white;font-size:14px;}"
                                      "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(70,193,243,1);}"
                                      "QPushButton:Pressed{border-radius:4px;background-color:rgba(27,160,213,1);}");
        ui->btnConnPWD->setEnabled(true);
    }
}

void OneConnForm::on_btnInfo_2_clicked(){
     on_btnInfo_clicked();
}

void OneConnForm::on_btnInfo_clicked()
{
    QPoint pos = QCursor::pos();
    QRect primaryGeometry;
    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(pos)) {
            primaryGeometry = screen->geometry();
        }
    }

    if (primaryGeometry.isEmpty()) {
        primaryGeometry = qApp->primaryScreen()->geometry();
    }

    BackThread *bt = new BackThread();
    QString connProp = bt->getConnProp(ui->lbName->text());
    QStringList propList = connProp.split("|");
    QString v4method, addr, mask, gateway, dns;
    foreach (QString line, propList) {
        if (line.startsWith("method:")) {
            v4method = line.split(":").at(1);
        }
        if (line.startsWith("addr:")) {
            addr = line.split(":").at(1);
        }
        if (line.startsWith("mask:")) {
            mask = line.split(":").at(1);
        }
        if (line.startsWith("gateway:")) {
            gateway= line.split(":").at(1);
        }
        if (line.startsWith("dns:")) {
            dns = line.split(":").at(1);
        }
    }
    // qDebug()<<"v4method:"<<v4method<<" addr:"<<addr<<" mask:"<<mask<<" gateway:"<<gateway<<" dns:"<<dns;

    cf->setProp(ui->lbName->text(), v4method, addr, mask, gateway, dns, this->isActive);

    cf->move(primaryGeometry.width() / 2 - cf->width() / 2, primaryGeometry.height() / 2 - cf->height() / 2);
    cf->show();
    cf->raise();

    bt->deleteLater();
}

// Wifi连接结果，0成功 1失败 2没有配置文件
void OneConnForm::slotConnWifiResult(int connFlag)
{
    mw->connWifiDone(connFlag);
    qDebug()<<"Function slotConnWifiResult receives a number: "<<connFlag;
    if (connFlag == 2) {
        mw->currSelNetName = "";
        emit selectedOneWifiForm(ui->lbName->text(), H_WIFI_ITEM_SMALL_EXTEND);

        resize(W_ITEM, H_ITEM_MIDDLE);
        ui->wbg->hide();
        ui->wbg_2->show();
        ui->wbg_3->hide();
        ui->leInfo_1->hide();
        ui->leInfo_2->hide();
        ui->leInfo_3->hide();
        ui->btnHideConn->hide();
        ui->btnDisConn->hide();
        ui->btnConn->hide();
        ui->btnConnSub->hide();
    //    ui->line->move(X_LINE_SMALL_EXTEND, Y_LINE_SMALL_EXTEND);

        ui->lePassword->show();
        ui->lePassword->setFocus();
        Q_EMIT onLineEditClicked();
        ui->checkBoxPwd->show();
        ui->btnConnPWD->show();
        ui->autoConn->show();
        ui->autoConn->setChecked(true);
        this->isSelected = true;
    }

    if (connFlag == 1) {
        // 使用配置文件连接失败，需要删除该配置文件
        QString txt(tr("Conn Wifi Failed"));//"连接 Wifi 失败"
        syslog(LOG_DEBUG, "Try to connect wifi named %s, but failed, will delete it's configuration file", ui->lbName->text().toUtf8().data());

        //YYF 20200922
//        ui->lePassword->setText("");

        KylinDBus kylindbus;
        kylindbus.showDesktopNotify(txt);
        //QString cmd = "export LANG='en_US.UTF-8';export LANGUAGE='en_US';nmcli connection delete '" + ui->lbName->text() + "';notify-send '" + txt + "...' -t 3800";
        QString name = ui->lbName->text();
        QString cmd = "export LANG='en_US.UTF-8';export LANGUAGE='en_US';nmcli connection delete \"" + name.replace("\"","\\\"") + "\"";
        int status = system(cmd.toUtf8().data());
        if (status != 0) {
            syslog(LOG_ERR, "execute 'nmcli connection delete' in function 'slotConnWifiResult' failed");
        }
//        ui->btnConnPWD->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(47, 179, 232, 1);color:white;font-size:14px;}"
//                                      "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(70,193,243,1);}"
//                                      "QPushButton:Pressed{border-radius:4px;background-color:rgba(27,160,213,1);}");
//        ui->btnConnPWD->setEnabled(true);
        ui->lePassword->setEnabled(true);
        ui->lePassword->setFocus();
        Q_EMIT onLineEditClicked();
        ui->lePassword->selectAll();
        ui->lePassword->setStyleSheet("QLineEdit{border:2px solid rgba(255, 0, 0, 0.5);border-radius:4px;"
                                      "background:palette(base);color:rgba(38, 38, 38, 1);font-size:14px;"
                                      "selection-background-color: rgba(255, 0, 0, 0.5);}");
    }
    if (connFlag == 0){
        //连接成功，配置是否自启动
        //若含中文的QString直接转换，会出现乱码，使用如下转换方式解决此问题（必须分两句转换）
        QString wifi_name = wifiName;
        wifi_name.replace("\"","\\\"");
        QByteArray ba = wifi_name.toLocal8Bit();
        char *name = ba.data();
        if (ui->autoConn->isVisible()) {
            //仅当首次连接配置时执行此操作
            kylin_network_set_autoconnect(name,ui->autoConn->isChecked());
        }
    }
    // 设置全局变量，当前连接Wifi的信号强度
    currentActWifiSignalLv = signalLv;

    this->stopWaiting();
    is_connecting = false;
    //if (connFlag != 0){
    //    mw->stopLoading();
    //}
}

void OneConnForm::waitAnimStep()
{
    //YYF 平板模式设计稿中没有此图标
//    QString qpmQss = "QLabel{background-image:url(':/res/s/conning-a/";
//    qpmQss.append(QString::number(this->waitPage));
//    qpmQss.append(".png');}");
//    ui->lbWaitingIcon->setStyleSheet(qpmQss);

//    this->waitPage --;
//    if (this->waitPage < 1) {
//        this->waitPage = TOTAL_PAGE; //循环播放8张图片
//    }

//    this->countCurrentTime += FRAME_SPEED;
//    if (this->countCurrentTime >= LIMIT_TIME) {
//        QString cmd = "kill -9 $(pidof nmcli)"; //杀掉当前正在进行的有关nmcli命令的进程
//        int status = system(cmd.toUtf8().data());
//        if (status != 0) {
//            qDebug()<<"execute 'kill -9 $(pidof nmcli)' in function 'waitAnimStep' failed";
//            syslog(LOG_ERR, "execute 'kill -9 $(pidof nmcli)' in function 'waitAnimStep' failed");
//        }

//        this->stopWaiting(); //动画超出时间限制，强制停止动画

//        mw->is_stop_check_net_state = 0;
//    }
}

void OneConnForm::startWaiting(bool isConn)
{
    //YYF 平板模式设计稿中没有此图标
//    this->isWaiting = true;
//    if (isConn) {
//        ui->lbWaiting->setStyleSheet("QLabel{border:0px;border-radius:10px;background-color:rgba(61,107,229,1);}");
//    } else {
//        ui->btnDisConn->hide();
//        ui->lbWaiting->setStyleSheet("QLabel{border:0px;border-radius:10px;background-color:rgba(255,255,255,0.12);}");
//    }
//    this->countCurrentTime = 0;
//    this->waitPage = TOTAL_PAGE; //总共有8张图片
//    this->waitTimer->start(FRAME_SPEED);
//    ui->lbWaiting->show();
//    ui->lbWaitingIcon->show();

//    mw->setTrayLoading(true);
}

void OneConnForm::stopWaiting()
{
    //YYF 平板模式设计稿中没有此图标
//    this->isWaiting = false;
//    this->waitTimer->stop();
//    ui->lbWaiting->hide();
//    ui->lbWaitingIcon->hide();

//    mw->setTrayLoading(false);
//    mw->getActiveInfo();
}
