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

#include "onelancform.h"
#include "ui_onelancform.h"
#include "kylinnm.h"

#include <time.h>

OneLancForm::OneLancForm(QWidget *parent, KylinNM *mainWindow, ConfForm *confForm, KSimpleNM *ksnm) :
    QWidget(parent),
    ui(new Ui::OneLancForm)
{
    ui->setupUi(this);

    ui->btnConnSub->setText(tr("Connect"));//"设置"
    ui->btnConn->setText(tr("Connect"));//"连接"
    ui->btnConn->hide();//YYF
    ui->btnDisConn->setText(tr("Disconnect"));//"断开连接"

    ui->lbConned->setAlignment(Qt::AlignLeft);

    leQssLow = "QLineEdit{border:none;background:transparent;font-size:14px;color:rgba(255,255,255,0.57);font-family:Noto Sans CJK SC;}";
    leQssHigh = "QLineEdit{border:none;background:transparent;font-size:14px;color:rgba(255,255,255,0.91);font-family:Noto Sans CJK SC;}";

    ui->leInfo_1->setStyleSheet(leQssLow);
    ui->leInfo_2->setStyleSheet(leQssLow);
    ui->leInfo_3->setStyleSheet(leQssLow);
    ui->leInfo_4->setStyleSheet(leQssLow);
    ui->btnInfo->setStyleSheet("QPushButton{border:none;background:transparent;}");
    ui->wbg->setStyleSheet("#wbg{border-radius:8px;background-color:rgba(255,255,255,0.1);border:1px solid red;}");//主窗口（展开）
    ui->wbg_2->setStyleSheet("#wbg_2{border-radius:8px;background-color:rgba(255,255,255,0);}");//列表窗口（闭合）
    ui->lbName->setStyleSheet("QLabel{font-size:14px;color:rgba(38, 38, 38, 1);}");//列表名称
    ui->lbConned->setStyleSheet("QLabel{font-size:14px;color:rgba(47, 179, 232, 1);}");//已连接
    ui->btnConnSub->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(61,107,229,1);color:white;font-size:14px;}"
                               "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(107,142,235,1);}"
                               "QPushButton:Pressed{border-radius:4px;background-color:rgba(50,87,202,1);}");
    ui->btnConn->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(61,107,229,1);color:white;font-size:14px;}"   //连接按钮
                               "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(107,142,235,1);}"
                               "QPushButton:Pressed{border-radius:4px;background-color:rgba(50,87,202,1);}");
//    ui->btnDisConn->setStyleSheet("QPushButton{border:0px;border-radius:4px;background-color:rgba(255,255,255,0.12);color:white;font-size:14px;}"
//                               "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(255,255,255,0.2);}"
//                               "QPushButton:Pressed{border-radius:8px;background-color:rgba(255,255,255,0.08);}");
    ui->btnDisConn->setStyleSheet("#btnDisConn{font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;"
                                  "font-weight: 400;color: rgba(38,38,38,0.75);line-height: 20px;font-size:14px;"
                                  "border:0px;background-color:rgba(38,38,38,0.1);border-radius:8px;}"
                                  "#btnDisConn:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:8px;background-color:rgba(38,38,38,0.2);}"
                                  "#btnDisConn:Pressed{border-radius:8px;background-color:rgba(38,38,38,0.06);}");//断开链接按钮
    ui->btnConnCfg->setStyleSheet("border:0px;border-radius:4px;border-image:url(:/res/l/pb-network-info.png);");//链接详情按钮
    connect(ui->btnConnCfg,&QPushButton::clicked,this,&OneLancForm::on_btnInfo_clicked);
    ui->btnConnCfg->hide();


    ui->btnInfo->setCursor(QCursor(Qt::PointingHandCursor));
    ui->btnInfo->setFocusPolicy(Qt::NoFocus);
    ui->btnConnSub->setFocusPolicy(Qt::NoFocus);
    ui->btnConn->setFocusPolicy(Qt::NoFocus);
    ui->btnDisConn->setFocusPolicy(Qt::NoFocus);

    ui->wbg->hide();
    ui->wbg_2->show();
    ui->lbName->show();
    ui->btnConnSub->hide();
    //YYF  ui->btnConn->hide();
    ui->btnDisConn->hide();
    ui->line->show();
    ui->lbWaiting->hide();
    ui->lbWaitingIcon->hide();

    this->mw = mainWindow;
    this->cf = confForm;
    this->ks = ksnm;

    this->isSelected = false;
    this->isActive = false;

    this->setAttribute(Qt::WA_Hover,true);
    this->installEventFilter(this);
    ui->btnInfo->setAttribute(Qt::WA_Hover,true);
    ui->btnInfo->installEventFilter(this);

    this->waitTimer = new QTimer(this);
    connect(waitTimer, SIGNAL(timeout()), this, SLOT(waitAnimStep()));

    connect(mw, SIGNAL(waitLanStop()), this, SLOT(stopWaiting()));

    srand((unsigned)time(NULL));

    ui->lbWaiting->setStyleSheet("QLabel{border:0px;border-radius:4px;background-color:rgba(61,107,229,1);}");
    //ui->lbWaitingIcon->setStyleSheet("QLabel{border:0px;background-color:transparent;}");
    ui->lbWaitingIcon->setStyleSheet("QLabel{border:0px;background-color:transparent;}");
    ui->lbWaitingIcon->move(316+194*mw->isTabletStyle,20);// YYF
    tabletStyle();//平板模式PC桌面样式
}

void OneLancForm::tabletStyle()//平板桌面模式特有设置
{
    ui->lbName->setFixedWidth(190+194*mw->isTabletStyle);
    ui->btnInfo->setFixedWidth(414+194*mw->isTabletStyle);
    ui->wbg_2->setFixedWidth(414+194*mw->isTabletStyle);
    ui->btnDisConn->move(316+194*mw->isTabletStyle,8 + 18*mw->isTabletStyle);
    ui->btnConnCfg->move(364+194*mw->isTabletStyle,18 + 18*mw->isTabletStyle);
}

OneLancForm::~OneLancForm()
{
    delete ui;
}

void OneLancForm::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    ui->wbg_2->setStyleSheet("#wbg_2{border-radius:8px;background-color:rgba(38,38,38,0.1);}");
}

void OneLancForm::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !rect().contains(event->pos())) {
        event->ignore();
        return;
    }
    if(ui->btnDisConn->isHidden()) {
        on_btnConn_clicked();//  YYF
    }
    ui->wbg_2->setStyleSheet("#wbg_2{border-radius:8px;background-color:rgba(255,255,255,0);}");
}

//事件过滤器
bool OneLancForm::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->btnInfo) {
        if(event->type() == QEvent::HoverEnter) {
            ui->leInfo_1->setStyleSheet(leQssHigh);
            ui->leInfo_2->setStyleSheet(leQssHigh);
            ui->leInfo_3->setStyleSheet(leQssHigh);
            ui->leInfo_4->setStyleSheet(leQssHigh);
            return true;
        } else if(event->type() == QEvent::HoverLeave) {
            ui->leInfo_1->setStyleSheet(leQssLow);
            ui->leInfo_2->setStyleSheet(leQssLow);
            ui->leInfo_3->setStyleSheet(leQssLow);
            ui->leInfo_4->setStyleSheet(leQssLow);
            return true;
        }
    } else if (obj == this) {
        if (event->type() == QEvent::HoverEnter) {
            if (!this->isTopItem) {
                if (!this->isSelected) {
                    //YYF  ui->btnConn->show();
                    ui->wbg_2->setStyleSheet("#wbg_2{border-radius:8px;background-color:rgba(38,38,38,0.05);}");
                    ui->wbg_2->show();
                }
            }
            return true;
        } else if(event->type() == QEvent::HoverLeave) {
            //YYF  ui->btnConn->hide();
            ui->wbg_2->setStyleSheet("#wbg_2{border-radius:8px;background-color:rgba(255,255,255,0);}");
            ui->wbg_2->hide();
            return true;
        }
    }

    return QWidget::eventFilter(obj,event);
}

// 是否当前连接的网络，字体设置不同
void OneLancForm::setAct(bool isAct)
{
    if (isAct) {
        ui->lbName->setStyleSheet("QLabel{font-size:14px;color:rgba(47, 179, 232, 1);}");
        ui->lbConned->show();
        ui->btnConnSub->hide();
    } else {
        ui->lbName->setStyleSheet("QLabel{font-size:14px;color:rgba(38, 38, 38, 0.45);}");
        ui->lbConned->hide();
        ui->btnConnSub->hide();
    }
    isActive = isAct;
}

// 是否选中
void OneLancForm::setSelected(bool isSelected, bool isCurrName)
{
    if (isSelected) {
        resize(W_ITEM, H_ITEM_EXTEND);
        ui->wbg->show();
        ui->wbg_2->hide();
        ui->line->move(X_LINE_EXTEND, Y_LINE_EXTEND);
        //YYF  ui->btnConn->hide();
        ui->btnConnSub->show();

        this->isSelected = true;
    } else {
        resize(W_ITEM, H_ITEM);
        ui->wbg->hide();
        ui->wbg_2->show();
        ui->line->move(X_LINE, Y_LINE);
        if(isCurrName){
            //YYF  ui->btnConn->show();
        }else{
            //YYF  ui->btnConn->hide();
        }
        ui->btnConnSub->hide();

        this->isSelected = false;
    }

    ui->btnDisConn->hide();

    this->isTopItem = false;
}

//设置顶部这个item的显示
void OneLancForm::setTopItem(bool isSelected)
{
//    if (isSelected) {
//        resize(W_ITEM, H_ITEM_EXTEND);
//        ui->wbg->show();
//        ui->btnConnSub->hide();
//        this->isSelected = true;
//    } else {
//        resize(W_ITEM, H_ITEM);
//        ui->wbg->hide();
//        ui->btnConnSub->hide();
//        this->isSelected = false;
//    }

    if (isConnected) {
        ui->lbIcon->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/l/pb-network-online.png);}");
        ui->btnDisConn->show();
//        ui->btnConnCfg->show();
    } else {
        ui->lbIcon->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/l/pb-top-network-offline.png);}");
//        ui->btnConnCfg->hide();
        ui->btnDisConn->hide();
    }
    ui->btnConnCfg->hide();

    //YYF  ui->btnConn->hide();
    ui->wbg_2->hide();
    ui->line->hide();

    this->isTopItem = true;
}

//设置网络名称
void OneLancForm::setName(QString name, QString uniName)
{
    ui->lbName->setText(name);
    lanName = name;
    uniqueName = uniName;
}

//根据有线网络连接与否，设置显示'已连接'文字的控件的可见与否
void OneLancForm::setConnedString(bool showLable, QString str)
{
    if (!showLable) {
        ui->lbConned->hide();
        ui->lbName->move(63, 18);
    } else {
        str="";//YYF  平板模式
        ui->lbConned->setText(str);
        ui->lbName->move(63, 18); //YYF
    }
}

//设置item被扩展后出现的网络信息
void OneLancForm::setLanInfo(QString str1, QString str2, QString str3, QString str4)
{
    if (str1 == "" || str1 == "auto") {
        str1 = tr("No Configuration");
    }

    if (str2 == "" || str2 == "auto") {
        str2 = tr("No Configuration");
    }

    QString strIPv4 = QString(tr("IPv4："));
    QString strIPv6 = QString(tr("IPv6："));
    QString strBW = QString(tr("BandWidth："));
    QString strMAC = QString(tr("MAC："));

    ui->leInfo_1->setText(strIPv4 + str1);
    ui->leInfo_2->setText(strIPv6 + str2);
    ui->leInfo_3->setText(strBW + str3);
    ui->leInfo_4->setText(strMAC + str4);
}

//根据网络是否连接，设置网络状态图标
void OneLancForm::setIcon(bool isOn)
{
    if (isOn) {
        ui->lbIcon->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/l/pb-network-online.png);}");
    } else {
        ui->lbIcon->setStyleSheet("QLabel{border-radius:0px;border-image:url(:/res/l/pb-network-offline.png);}");
    }
}

//设置item下方横线的可见与否
void OneLancForm::setLine(bool isShow)
{
    if (isShow) {
        ui->line->show();
    } else {
        ui->line->hide();
    }
}

void OneLancForm::slotConnLan()
{
    //mw->startLoading();
    this->startWaiting(true);
    emit sigConnLan(ui->lbName->text());
}

//点击网络断开按钮，执行该函数
void OneLancForm::on_btnDisConn_clicked()
{
    syslog(LOG_DEBUG, "DisConnect button about lan net is clicked, current wired net name is %s .", ui->lbName->text().toUtf8().data());
    qDebug()<<"DisConnect button about lan net is clicked, current wired net name is "<<ui->lbName->text();

    this->startWaiting(false);
    mw->is_stop_check_net_state = 1;

    kylin_network_set_con_down(ui->lbName->text().toUtf8().data());

    disconnect(this, SIGNAL(selectedOneLanForm(QString, QString)), mw, SLOT(oneTopLanFormSelected(QString, QString)));

    emit disconnActiveLan();
}

//点击了连接网络按钮，执行该函数
void OneLancForm::on_btnConn_clicked()
{
    syslog(LOG_DEBUG, "A button named btnConn about lan net is clicked.");
    qDebug()<<"A button named btnConn about lan net is clicked.";
    toConnectWiredNetwork();
}

//点击了item被扩展中的连接网络按钮，执行该函数
void OneLancForm::on_btnConnSub_clicked()
{
    syslog(LOG_DEBUG, "A button named btnConnSub about lan net is clicked.");
    qDebug()<<"A button named btnConnSub about lan net is clicked.";
    toConnectWiredNetwork();
}

void OneLancForm::toConnectWiredNetwork()
{
    mw->is_stop_check_net_state = 1;
    QThread *t = new QThread();
    BackThread *bt = new BackThread();
    bt->moveToThread(t);
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
    connect(t, SIGNAL(started()), this, SLOT(slotConnLan()));
    connect(this, SIGNAL(sigConnLan(QString)), bt, SLOT(execConnLan(QString)));
//    connect(bt, SIGNAL(connDone(int)), mw, SLOT(connLanDone(int)));
    connect(bt, &BackThread::connDone, this, [=](int f) {
        mw->connLanDone(f);
    });
    connect(bt, SIGNAL(btFinish()), t, SLOT(quit()));
    t->start();
}

//点击列表中item扩展后显示信息的位置时，执行该函数，用于显示网络配置界面
void OneLancForm::on_btnInfo_clicked()
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
    // qDebug()<<v4method<<addr<<mask<<gateway<<dns;

    cf->setProp(ui->lbName->text(), v4method, addr, mask, gateway, dns, this->isActive);
    connect(cf, SIGNAL(requestRefreshLanList(int)), mw, SLOT(onBtnNetListClicked(int)));

    cf->move(primaryGeometry.width() / 2 - cf->width() / 2, primaryGeometry.height() / 2 - cf->height() / 2);
    cf->show();
    cf->raise();
    cf->activateWindow();
}

void OneLancForm::waitAnimStep()
{
    QString qpmQss = "QLabel{background-image:url(':/res/s/conning-a/";
    qpmQss.append(QString::number(this->waitPage));
    qpmQss.append(".png');}");
    ui->lbWaitingIcon->setStyleSheet(qpmQss);

    this->waitPage --;
    if (this->waitPage < 1) {
        this->waitPage = TOTAL_PAGE; //循环播放8张图片
    }

    this->countCurrentTime += FRAME_SPEED;
    if (this->countCurrentTime >= LIMIT_TIME) {
        QString cmd = "kill -9 $(pidof nmcli)"; //杀掉当前正在进行的有关nmcli命令的进程
        int status = system(cmd.toUtf8().data());
        if (status != 0) {
            qDebug()<<"execute 'kill -9 $(pidof nmcli)' in function 'waitAnimStep' failed";
            syslog(LOG_ERR, "execute 'kill -9 $(pidof nmcli)' in function 'waitAnimStep' failed");
        }

        this->stopWaiting(); //动画超出时间限制，强制停止动画

        mw->is_stop_check_net_state = 0;
    }
}

void OneLancForm::startWaiting(bool isConn)
{
    if (isConn) {
        ui->lbWaiting->setStyleSheet("QLabel{border:0px;border-radius:4px;background-color:rgba(61,107,229,1);}");
    } else {
        ui->btnDisConn->hide();
        ui->lbWaiting->setStyleSheet("QLabel{border:0px;border-radius:4px;background-color:rgba(255,255,255,0.12);}");
    }
    this->countCurrentTime = 0;
    this->waitPage = TOTAL_PAGE; //总共有8张图片
    this->waitTimer->start(FRAME_SPEED);
//    ui->lbWaiting->show();  YYF
    ui->lbWaitingIcon->show();

    mw->setTrayLoading(true);
}

void OneLancForm::stopWaiting()
{
    this->waitTimer->stop();
    ui->lbWaiting->hide();
    ui->lbWaitingIcon->hide();

    mw->setTrayLoading(false);
    mw->getActiveInfo();
}
