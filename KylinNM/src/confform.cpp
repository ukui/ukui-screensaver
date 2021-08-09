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

#include "confform.h"
#include "ui_confform.h"
#include "kylin-network-interface.h"
#include "backthread.h"
#include "utils.h"
#include "kylinnm.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

extern QString llname, lwname;

ConfForm::ConfForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfForm)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint); //Qt::WindowStaysOnTopHint
    this->setWindowTitle(tr("edit network"));//"网络设置"
    this->setAttribute(Qt::WA_TranslucentBackground);
    //需要添加 void paintEvent(QPaintEvent *event) 函数

    QPainterPath path;
    auto rect = this->rect();
    rect.adjust(1, 1, -1, -1);
    path.addRoundedRect(rect, 6, 6);
    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));

    this->setStyleSheet("QWidget{border-radius:16px;border:none;}");
//    KylinDBus mkylindbus;

    QString strTrans;
    strTrans =  QString::number(1, 10, 2);
    QString sty = "#centralWidget{border:1px solid rgba(255,255,255,0.05);border-radius:16px;background:rgba(255,255,255," + strTrans + ");}";
    ui->centralWidget->setStyleSheet(sty);

    labelQss = "QLabel{border:0px;color:rgba(38,38,38,1);background-color:transparent;}";
    cbxQss = "QComboBox{padding-left:20px;font-size:13px;color:rgba(38,38,38,1);"
             "border:1px solid rgba(255, 255, 255, 0.05);border-radius:4px;background:rgba(255,255,255,0.08);}"
             "QComboBox::drop-down{border:0px;width:30px;}"
             "QComboBox::down-arrow{image:url(:/res/g/down_arrow.png);}"
             "QComboBox QAbstractItemView {margin:0px 0px 0px 0px;padding: 0px 0px;border-radius:0px;background-color:#48484C;outline:0px;}"
             "QComboBox QAbstractItemView::item{padding-left:17px;border-radius:0px;font-size:13px;color:rgba(255,255,255,0.91);height: 32px;background-color:#48484C;outline:0px;}"
             "QComboBox QAbstractItemView::item:hover{padding-left:17px;border-radius:0px;font-size:13px;color:rgba(255,255,255,0.91);background-color:#3D6BE5;outline:0px;}";
    leQss = "QLineEdit{padding-left:20px;color:rgba(38,38,38,1);background:rgba(245,245,245,1);}";
    btnOffQss = "QPushButton{border:1px solid rgba(44,44,44,1);border-radius:4px;background-color:rgba(255,255,255,0.12);color:rgba(44,44,44,1);font-size:14px;}";
    //                   "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(255,255,255,0.2);}"
    //                   "QPushButton:Pressed{border-radius:4px;background-color:rgba(255,255,255,0.08);}";
    btnOnQss = "QPushButton{border:0px;border-radius:4px;background-color:rgba(61,107,229,1);color:white;font-size:14px;}"
               "QPushButton:Hover{border:0px solid rgba(255,255,255,0.2);border-radius:4px;background-color:rgba(107,142,235,1);}"
               "QPushButton:Pressed{border-radius:4px;background-color:rgba(50,87,202,1);}";
    lineQss = "background:rgba(255,255,255,0.08);";

    // ui->centralWidget->setStyleSheet("#centralWidget{border:1px solid #297a97;background-color:#ffffff;}");
    ui->wdHead->setStyleSheet("#wdHead{border:none}");
    ui->wgManual->setStyleSheet("#wgManual{border:none}");
    ui->wdBottom->setStyleSheet("#wdBottom{border:none}");
    ui->lbLeftupTitle->setStyleSheet("QLabel{border:0px;font-size:14px;color:rgba(38,38,38,1);background-color:transparent;}");
    ui->lbName->setStyleSheet(labelQss);
    ui->lbTxt1->setStyleSheet(labelQss);
    ui->lbTxt2->setStyleSheet(labelQss);
    ui->lbTxt3->setStyleSheet(labelQss);
    ui->lbTxt4->setStyleSheet(labelQss);
    ui->lbTxt5->setStyleSheet(labelQss);
    ui->lbTxt6->setStyleSheet(labelQss);

    //YYF
    ui->btnClose->setStyleSheet("border-image:url(:/res/x/pb-close.png);");
    connect(ui->btnClose,&QPushButton::clicked,ui->btnCancel,&QPushButton::clicked);
    ui->isDHCP->setStyleSheet("QCheckBox::indicator {width: 16px; height: 16px;}"
                              "QCheckBox::indicator:checked {image: url(:/res/g/checkbox-checked.svg);}"
                              "QCheckBox::indicator:unchecked {image: url(:/res/g/checkbox-unchecked.svg);}");
    ui->notDHCP->setStyleSheet("QCheckBox::indicator {width: 16px; height: 16px;}"
                               "QCheckBox::indicator:checked {image: url(:/res/g/checkbox-checked.svg);}"
                               "QCheckBox::indicator:unchecked {image: url(:/res/g/checkbox-unchecked.svg);}");

    ui->cbType->setStyleSheet(cbxQss);
    ui->cbType->setView(new  QListView());
    ui->cbType->hide();
    ui->cbMask->setStyleSheet(cbxQss);
    ui->cbMask->setView(new  QListView());

    ui->leName->setStyleSheet(leQss);
    ui->leAddr->setStyleSheet(leQss);
    ui->leGateway->setStyleSheet(leQss);
    ui->leDns->setStyleSheet(leQss);
    ui->leDns2->setStyleSheet(leQss);

    ui->btnCancel->setStyleSheet(btnOffQss);
    ui->btnOk->setStyleSheet(btnOnQss);
    ui->btnCreate->setStyleSheet(btnOnQss);
    ui->lineUp->setStyleSheet(lineQss);
    ui->lineDown->setStyleSheet(lineQss);
    ui->lineUp->hide();
    ui->lineDown->hide();

    ui->lbName->setText(tr("LAN name: "));//"网络名称："
    ui->lbTxt1->setText(tr("Method: "));//"编辑IP设置："
    ui->lbTxt2->setText(tr("Address: "));//"IP地址："
    ui->lbTxt3->setText(tr("Netmask: "));//"子网掩码："
    ui->lbTxt4->setText(tr("Gateway: "));//"默认网关："
    ui->lbTxt5->setText(tr("DNS 1: "));//"首选DNS："
    ui->lbTxt6->setText(tr("DNS 2: "));//"备选DNS："

    ui->lbLeftupTitle->setText(tr("Edit Conn"));//"网络设置"
    ui->cbType->addItem(tr("Auto(DHCP)"));//"自动(DHCP)"
    ui->cbType->addItem(tr("Manual"));//"手动"
    ui->isDHCP->setText(tr("Auto(DHCP)"));
    ui->notDHCP->setText(tr("Manual"));

    connect(ui->cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(cbTypeChanged(int)));

    ui->cbMask->addItem("255.255.255.0"); //24
    ui->cbMask->addItem("255.255.254.0"); //23
    ui->cbMask->addItem("255.255.252.0"); //22
    ui->cbMask->addItem("255.255.0.0"); //16
    ui->cbMask->addItem("255.0.0.0"); //8
//    ui->cbMask->setWindowFlags(Qt::X11BypassWindowManagerHint);
    ui->cbMask->view()->setWindowFlags(Qt::Popup | Qt::X11BypassWindowManagerHint);
    ui->cbMask->view()->setParent(this);
    ui->cbMask->view()->hide();

    ui->cbMask->installEventFilter(this);
    ui->cbMask->view()->installEventFilter(this);

    connect(ui->cbMask->view(), &QAbstractItemView::pressed, this, [=](QModelIndex index){
//       qDebug() << "-----------------------pressed:" << index.row();
//       qDebug() << "-----------------------itemText:" << ui->cbMask->itemText(index.row());
       Q_EMIT ui->cbMask->setCurrentIndex(index.row());
       ui->cbMask->view()->hide();
    });

//    connect(ui->cbMask->view(), &QAbstractItemView::entered, this, [=](QModelIndex index){
//       qDebug() << "-----------------------entered:" << index.row();
//    });

    ui->cbMask->view()->setGeometry(QRect(ui->cbMask->geometry().left(), ui->wgManual->geometry().top() + ui->cbMask->geometry().bottom(), ui->cbMask->view()->width(), ui->cbMask->view()->height()));

    ui->btnCancel->setText(tr("Cancel"));//"取消"
    ui->btnOk->setText(tr("Save"));//"保存"
    ui->btnCreate->setText(tr("Ok"));//"确定"

    ui->btnCancel->setFocusPolicy(Qt::NoFocus);
    ui->btnOk->setFocusPolicy(Qt::NoFocus);
    ui->btnCreate->setFocusPolicy(Qt::NoFocus);

    // IP的正则格式限制
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    ui->leAddr->setValidator(new QRegExpValidator(rx, this));
    ui->leGateway->setValidator(new QRegExpValidator(rx, this));
    ui->leDns->setValidator(new QRegExpValidator(rx, this));
    ui->leDns2->setValidator(new QRegExpValidator(rx, this));

}

void ConfForm::setMainWindow(KylinNM *mainwindow)
{
    this->mw = mainwindow;
}

ConfForm::~ConfForm()
{
    delete ui;
}

bool ConfForm::eventFilter(QObject *obj, QEvent *ev)
{
    if(obj == ui->cbMask)
    {
        if(ev->type() == QEvent::MouseButtonPress)
        {
            ui->cbMask->view()->setVisible(!ui->cbMask->view()->isVisible());
            if(ui->cbMask->view()->isVisible())
                ui->cbMask->view()->setFocus();
        }
    } else if (obj != ui->cbMask->view())
    {
        ui->cbMask->view()->hide();
    }
    return false;
}

void ConfForm::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->isPress = true;
        this->winPos = this->pos();
        this->dragPos = event->globalPos();
        event->accept();
    }
}
void ConfForm::mouseReleaseEvent(QMouseEvent *event)
{
    this->isPress = false;
}
void ConfForm::mouseMoveEvent(QMouseEvent *event)
{
    if (this->isPress) {
        this->move(this->winPos - (this->dragPos - event->globalPos()));
        event->accept();
    }
}

void ConfForm::tabletStyle(bool isTablet)
{
    isTabletStyle = isTablet;

    ui->leName->resize(290+194*isTabletStyle,48);
    ui->leName->resize(290+194*isTabletStyle,48);
    ui->btnOk->move(312+194*isTabletStyle,25);
    ui->btnCreate->move(312+194*isTabletStyle,25);
    ui->btnCancel->move(180+194*isTabletStyle,25);
    ui->btnClose->move(408+194*isTabletStyle,8);
    ui->isDHCP->move(180+194*isTabletStyle,130);
    ui->notDHCP->move(356+194*isTabletStyle,130);

    ui->wdBottom->setFixedWidth(446+194*isTabletStyle);
    ui->wdHead->setFixedWidth(446+194*isTabletStyle);
    ui->wgManual->setFixedWidth(446+194*isTabletStyle);
    ui->upTitle->setFixedWidth(446+194*isTabletStyle);
}

//网络配置参数设置界面的显示内容
void ConfForm::setProp(QString connName, QString v4method, QString addr, QString mask, QString gateway, QString dns, bool isActConf)
{
    this->isActConf = isActConf;
    ui->leName->setText(connName);

    if (v4method == "auto" || v4method == "") {
        ui->isDHCP->setChecked(true);
        ui->notDHCP->setChecked(false);
        ui->cbType->setCurrentIndex(0);
        cbTypeChanged(0);
    } else {
        ui->isDHCP->setChecked(false);
        ui->notDHCP->setChecked(true);
        ui->cbType->setCurrentIndex(1);
        cbTypeChanged(1);
    }

    ui->leAddr->setText(addr);
    ui->leGateway->setText(gateway);

    // 配置中有多个DNS，只处理前两个
    if (dns.indexOf(",") != -1) {
        QStringList dnss = dns.split(",");
        ui->leDns->setText(dnss.at(0));
        ui->leDns2->setText(dnss.at(1));
    } else {
        ui->leDns->setText(dns);
        ui->leDns2->setText("");
    }

    if (mask == "24") {
        ui->cbMask->setCurrentIndex(0);
    } else if(mask == "23") {
        ui->cbMask->setCurrentIndex(1);
    } else if(mask == "22") {
        ui->cbMask->setCurrentIndex(2);
    } else if(mask == "16") {
        ui->cbMask->setCurrentIndex(3);
    } else if(mask == "8") {
        ui->cbMask->setCurrentIndex(4);
    } else {
        ui->cbMask->setCurrentIndex(0);
    }
}

//点击了创建新的网络的按钮
void ConfForm::on_btnCreate_clicked()
{
    if (mw->lanNameList.contains(ui->leName->text())) {
        qWarning() << "Warn : 已有同名LAN存在:" << ui->leName->text();
        //            ui->tipLabel->show();
        return;
    }
    KylinDBus kylindbus;
    kylindbus.getWiredCardName();
    QString mIfname = kylindbus.dbusLanCardName;

    if (mIfname == "") {
        QString tip(tr("Can not create new wired network for without wired card"));
//        kylindbus.showDesktopNotify(tip);
        this->hide();
        return;
    }

    QString name = ui->leName->text();
    QString cmdStr = "nmcli connection add con-name \"" + name.replace("\"","\\\"") + "\" type ethernet"; //由于样机存在veth虚拟网卡，故不指定有线设备，将自动选择
    Utils::m_system(cmdStr.toUtf8().data());
    //int status = system(cmdStr.toUtf8().data());
    //if (status != 0){ syslog(LOG_ERR, "execute 'nmcli connection add con-name' in function 'on_btnCreate_clicked' failed");}

    if (ui->cbType->currentIndex() == 1) {
        //config the ipv4 and netmask and gateway if select Manual
        this->isCreateNewNet = true;
        this->on_btnOk_clicked();
    } else {
        QString txt(tr("New network already created"));
//        kylindbus.showDesktopNotify(txt);
    }

    this->hide();
}

//点击了保存更改网络设置的按钮
void ConfForm::on_btnOk_clicked()
{
    QString mask = "";
    if (ui->cbMask->currentIndex() == 0) {
        mask = "24";
    } else if(ui->cbMask->currentIndex() == 1) {
        mask = "23";
    } else if(ui->cbMask->currentIndex() == 2) {
        mask = "22";
    } else if(ui->cbMask->currentIndex() == 3) {
        mask = "16";
    } else if(ui->cbMask->currentIndex() == 4) {
        mask = "8";
    } else {
        mask = "24";
    }

    if (ui->cbType->currentIndex() == 0) {
        QString name = ui->leName->text();
        name.replace("\"","\\\"");
        kylin_network_set_automethod(name.toUtf8().data());
    } else {
        QString dnss = ui->leDns->text();
        if (ui->leDns2->text() != "") {
            dnss.append(",");
            dnss.append(ui->leDns2->text());
        }
        QString name = ui->leName->text();
        name.replace("\"","\\\"");
        kylin_network_set_manualall(name.toUtf8().data(), ui->leAddr->text().toUtf8().data(), mask.toUtf8().data(), ui->leGateway->text().toUtf8().data(), dnss.toUtf8().data());
    }

    KylinDBus kylindbus;

    this->hide();

    QString txt(tr("New network settings already finished"));
    kylindbus.showDesktopNotify(txt);

    if (!this->isCreateNewNet) {
        if (this->isActConf == true) {
            // 如果是修改当前连接的网络，则修改设置后简略重连网络
            //QString cmd = "/usr/share/kylin-nm/shell/connup.sh '" + ui->leName->text() + "'";
            kylindbus.connectWiredNet(ui->leName->text()); //reconnect this wired network

            QString m_txt(tr("New settings already effective"));
            kylindbus.showDesktopNotify(m_txt); //show desktop notify
        }

        //需要更新一下有线网界面
        qDebug()<<"debug: request refresh Lan list";
        emit requestRefreshLanList(0);
    }
    this->isCreateNewNet = false;
}

//点击取消按钮
void ConfForm::on_btnCancel_clicked()
{
    this->hide();
}

//根据需要设置的种类(自动或手动等)显示界面内容
void ConfForm::cbTypeChanged(int index)
{
    if (isShowSaveBtn) {
        ui->leName->setEnabled(false);
        ui->btnOk->show(); //显示保存按钮
        ui->btnCreate->hide(); //隐藏创建按钮
        ui->lbLeftupTitle->setText(tr("Edit Network"));
    }

    if (index == 0) {
        ui->lineUp->hide();
        ui->lineDown->hide();
        ui->wgManual->hide();
        ui->centralWidget->resize(446+194*isTabletStyle, 443);
        ui->wdBottom->move(0, 347);

        this->setEnableOfBtn();

        this->resize(446+194*isTabletStyle,443);
    }
    if (index == 1) {
        ui->lineUp->show();
        ui->lineDown->show();
        ui->wgManual->show();
        ui->centralWidget->resize(446+194*isTabletStyle, 576);
        ui->wdBottom->move(0, 480);

        this->setEnableOfBtn();

        this->resize(446+194*isTabletStyle, 576);
    }
    if (index == 3) {
        ui->isDHCP->setChecked(true);
        ui->notDHCP->setChecked(false);
        ui->btnOk->setStyleSheet(btnOffQss);
        ui->btnOk->setEnabled(false);
        ui->btnCreate->setStyleSheet(btnOffQss);
        ui->btnCreate->setEnabled(false);

        ui->leName->setEnabled(true);
        ui->btnOk->hide();
        ui->btnCreate->show();
        ui->lbLeftupTitle->setText(tr("Add Wired Network"));
        isShowSaveBtn = false;

        ui->lineUp->hide();
        ui->lineDown->hide();
        ui->wgManual->hide();
        ui->centralWidget->resize(446+194*isTabletStyle, 443);
        ui->wdBottom->move(0, 347);
        this->resize(446+194*isTabletStyle, 443);
    }
}

//编辑网络名称
void ConfForm::on_leName_textEdited(const QString &arg1)
{
    this->setEnableOfBtn();
}

//编辑网络ip
void ConfForm::on_leAddr_textEdited(const QString &arg1)
{
    this->setEnableOfBtn();
}

//编辑网络网关
void ConfForm::on_leGateway_textEdited(const QString &arg1)
{
    this->setEnableOfBtn();
}

//编辑网络DNS
void ConfForm::on_leDns_textEdited(const QString &arg1)
{
    this->setEnableOfBtn();
}

//编辑网络备用DNS
void ConfForm::on_leDns2_textEdited(const QString &arg1)
{
    // this->setEnableOfBtn();
}

//设置界面按钮是否可点击
void ConfForm::setEnableOfBtn()
{
    if (ui->leName->text().size() == 0 ) {
        this->setBtnEnableFalse();
        return;
    }

    if (ui->cbType->currentIndex() == 1) {
        if (!this->getTextEditState(ui->leAddr->text()) ) {
            this->setBtnEnableFalse();
            return;
        }

        if (!this->getTextEditState(ui->leGateway->text()) ) {
            this->setBtnEnableFalse();
            return;
        }

        if (!this->getTextEditState(ui->leDns->text()) ) {
            this->setBtnEnableFalse();
            return;
        }
    }

    ui->btnOk->setStyleSheet(btnOnQss);
    ui->btnOk->setEnabled(true);

    ui->btnCreate->setStyleSheet(btnOnQss);
    ui->btnCreate->setEnabled(true);
}

//文本的输入要符合ip的格式要求
bool ConfForm::getTextEditState(QString text)
{
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

    bool match = false;
    match = rx.exactMatch(text);
    // qDebug()<<"the match result is: " << match;

    return match;
}

//设置创建或保存按钮不可点击
void ConfForm::setBtnEnableFalse()
{
    ui->btnOk->setStyleSheet(btnOffQss);
    ui->btnOk->setEnabled(false);

    ui->btnCreate->setStyleSheet(btnOffQss);
    ui->btnCreate->setEnabled(false);
}

void ConfForm::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

void ConfForm::on_isDHCP_clicked()
{
    ui->isDHCP->setEnabled(false);
    ui->notDHCP->setEnabled(true);
    ui->notDHCP->setChecked(false);
    ui->cbType->setCurrentIndex(0);
    cbTypeChanged(0);
}

void ConfForm::on_notDHCP_clicked()
{
    ui->notDHCP->setEnabled(false);
    ui->isDHCP->setEnabled(true);
    ui->isDHCP->setChecked(false);
    ui->cbType->setCurrentIndex(1);
    cbTypeChanged(1);
}
