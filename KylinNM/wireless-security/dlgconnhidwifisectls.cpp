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


#include "ui_dlgconnhidwifisectls.h"
#include "kylinheadfile.h"

#include <sys/syslog.h>

DlgConnHidWifiSecTls::DlgConnHidWifiSecTls(int type, QWidget *parent) :
    WepOrWpa(type),
    QDialog(parent),
    ui(new Ui::DlgConnHidWifiSecTls)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    //需要添加 void paintEvent(QPaintEvent *event) 函数

    QPainterPath path;
    auto rect = this->rect();
    rect.adjust(1, 1, -1, -1);
    path.addRoundedRect(rect, 6, 6);
    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));

    this->setStyleSheet("QWidget{border-radius:6px;background-color:rgba(19,19,20,0.7);border:1px solid rgba(255, 255, 255, 0.05);}");

    MyQss objQss;

    ui->lbBoder->setStyleSheet("QLabel{border-radius:6px;background-color:rgba(19,19,20,0.95);border:1px solid rgba(255, 255, 255, 0.05);}");
    ui->lbBoder->hide();
    ui->lbLeftupTitle->setStyleSheet("QLabel{border:0px;font-size:20px;color:rgba(255,255,255,0.97);background-color:transparent;}");
    ui->lbConn->setStyleSheet(objQss.labelQss);
    ui->lbNetName->setStyleSheet(objQss.labelQss);
    ui->lbSecurity->setStyleSheet(objQss.labelQss);
    ui->lbAuth->setStyleSheet(objQss.labelQss);
    ui->lbIdentity->setStyleSheet(objQss.labelQss);
    ui->lbDomain->setStyleSheet(objQss.labelQss);
    ui->lbCA->setStyleSheet(objQss.labelQss);
    ui->lbCaPwd->setStyleSheet(objQss.labelQss);
    ui->lbUserCertify->setStyleSheet(objQss.labelQss);
    ui->lbUserCertifyPwd->setStyleSheet(objQss.labelQss);
    ui->lbUserPriKey->setStyleSheet(objQss.labelQss);
    ui->lbUserKeyPwd->setStyleSheet(objQss.labelQss);

    ui->cbxConn->setStyleSheet(objQss.cbxQss);
    ui->cbxConn->setView(new  QListView());
    ui->leNetName->setStyleSheet(objQss.leQss);
    ui->cbxSecurity->setStyleSheet(objQss.cbxQss);
    ui->cbxSecurity->setView(new  QListView());
    ui->cbxAuth->setStyleSheet(objQss.cbxQss);
    ui->cbxAuth->setView(new  QListView());
    ui->leIdentity->setStyleSheet(objQss.leQss);
    ui->leDomain->setStyleSheet(objQss.leQss);
    ui->cbxCA->setStyleSheet(objQss.cbxQss);
    ui->cbxCA->setView(new  QListView());
    ui->leCaPwd->setStyleSheet(objQss.leQss);
    ui->checkBoxPwd->setStyleSheet(objQss.checkBoxQss);
    ui->checkBoxCA->setStyleSheet(objQss.checkBoxCAQss);
    ui->cbxUserCertify->setStyleSheet(objQss.cbxQss);
    ui->cbxUserCertify->setView(new  QListView());
    ui->leUserCertifyPwd->setStyleSheet(objQss.leQss);
    ui->cbxUserPriKey->setStyleSheet(objQss.cbxQss);
    ui->cbxUserPriKey->setView(new  QListView());
    ui->leUserKeyPwd->setStyleSheet(objQss.leQss);
    ui->checkBoxPwdSec->setStyleSheet(objQss.checkBoxQss);

    ui->btnCancel->setStyleSheet(objQss.btnCancelQss);
    ui->btnConnect->setStyleSheet(objQss.btnCancelQss);
    ui->lineUp->setStyleSheet(objQss.lineQss);
    ui->lineDown->setStyleSheet(objQss.lineQss);
    ui->checkBoxCA->setFocusPolicy(Qt::NoFocus);

    ui->lbLeftupTitle->setText(tr("Add hidden Wi-Fi")); //加入隐藏Wi-Fi
    ui->lbConn->setText(tr("Connection")); //连接设置:
    ui->lbNetName->setText(tr("Network name")); //网络名称:
    ui->lbSecurity->setText(tr("Wi-Fi security")); //Wi-Fi安全性:
    ui->lbAuth->setText(tr("Authentication")); //认证:
    ui->lbIdentity->setText(tr("Identity")); //身份:
    ui->lbDomain->setText(tr("Domain")); //域名:
    ui->lbCA->setText(tr("CA certificate")); //CA 证书:
    ui->lbCaPwd->setText(tr("CA certificate password")); //CA 证书密码:
    ui->checkBoxCA->setText(tr("No CA certificate is required")); //不需要CA证书
    ui->lbUserCertify->setText(tr("User certificate")); //用户证书:
    ui->lbUserCertifyPwd->setText(tr("User certificate password")); //用户证书密码:
    ui->lbUserPriKey->setText(tr("User private key")); //用户私钥:
    ui->lbUserKeyPwd->setText(tr("User key password")); //用户密钥密码:
    ui->btnCancel->setText(tr("Cancel")); //取消
    ui->btnConnect->setText(tr("Connect")); //连接

    ui->cbxConn->addItem(tr("C_reate…")); //新建...
    int status = system("nmcli connection show>/tmp/kylin-nm-connshow");
    if (status != 0){ syslog(LOG_ERR, "execute 'nmcli connection show' in function 'DlgConnHidWifiSecTls' failed");}
    QFile file("/tmp/kylin-nm-connshow");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Can't open the file!";
    }
    QString txt = file.readAll();
    QStringList txtLine = txt.split("\n");
    file.close();
    foreach (QString line, txtLine) {
        if(line.indexOf("wifi") != -1){
            QStringList subLine = line.split(" ");
            ui->cbxConn->addItem(subLine[0]);
        }
    }
    ui->cbxConn->setCurrentIndex(0);

    ui->cbxSecurity->addItem(tr("None")); //无
    ui->cbxSecurity->addItem(tr("WPA & WPA2 Personal")); //WPA 及 WPA2 个人
    ui->cbxSecurity->addItem(tr("WEP 40/128-bit Key (Hex or ASCII)")); //WEP 40/128 位密钥(十六进制或ASCII)
    ui->cbxSecurity->addItem(tr("WEP 128-bit Passphrase")); //WEP 128 位密码句
    ui->cbxSecurity->addItem("LEAP");
    ui->cbxSecurity->addItem(tr("Dynamic WEP (802.1X)")); //动态 WEP (802.1x)
    ui->cbxSecurity->addItem(tr("WPA & WPA2 Enterprise")); //WPA 及 WPA2 企业
    if (WepOrWpa == 0) {
        ui->cbxSecurity->setCurrentIndex(5);
    } else if (WepOrWpa == 1) {
        ui->cbxSecurity->setCurrentIndex(6);
    }
    connect(ui->cbxSecurity,SIGNAL(currentIndexChanged(QString)),this,SLOT(changeDialogSecu()));

    ui->cbxAuth->addItem("TLS");
    ui->cbxAuth->addItem("LEAP");
    ui->cbxAuth->addItem("PWD");
    ui->cbxAuth->addItem("FAST");
    ui->cbxAuth->addItem(tr("Tunneled TLS"));//隧道 TLS
    ui->cbxAuth->addItem(tr("Protected EAP (PEAP)")); //受保护的 EAP
    ui->cbxAuth->setCurrentIndex(0);
    connect(ui->cbxAuth,SIGNAL(currentIndexChanged(QString)),this,SLOT(changeDialogAuth()));

    ui->cbxCA->addItem(tr("None")); //无
    ui->cbxCA->addItem(tr("Choose from file")); //从文件选择...
    ui->cbxCA->setCurrentIndex(0);

    ui->cbxUserCertify->addItem(tr("None")); //无
    ui->cbxUserCertify->addItem(tr("Choose from file")); //从文件选择...
    ui->cbxUserCertify->setCurrentIndex(0);

    ui->cbxUserPriKey->addItem(tr("None")); //无
    ui->cbxUserPriKey->addItem(tr("Choose from file")); //从文件选择...
    ui->cbxUserPriKey->setCurrentIndex(0);

    ui->btnConnect->setEnabled(false);

    ui->lbCaPwd->setEnabled(false);
    ui->leCaPwd->setEnabled(false);

    ui->lbUserCertifyPwd->setEnabled(false);
    ui->leUserCertifyPwd->setEnabled(false);

    ui->lbUserPriKey->setEnabled(false);
    ui->cbxUserPriKey->setEnabled(false);

    ui->lbUserKeyPwd->setEnabled(false);
    ui->leUserKeyPwd->setEnabled(false);

    this->setFixedSize(432,705);

}

DlgConnHidWifiSecTls::~DlgConnHidWifiSecTls()
{
    delete ui;
}

void DlgConnHidWifiSecTls::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        this->isPress = true;
        this->winPos = this->pos();
        this->dragPos = event->globalPos();
        event->accept();
    }
}
void DlgConnHidWifiSecTls::mouseReleaseEvent(QMouseEvent *event){
    this->isPress = false;
}
void DlgConnHidWifiSecTls::mouseMoveEvent(QMouseEvent *event){
    if(this->isPress){
        this->move(this->winPos - (this->dragPos - event->globalPos()));
        event->accept();
    }
}

void DlgConnHidWifiSecTls::changeDialogSecu()
{
    if(ui->cbxSecurity->currentIndex()==0){
//        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifi *connHidWifi = new DlgConnHidWifi(0, 0, this->parentWidget());
        connHidWifi->show();
    } else if(ui->cbxSecurity->currentIndex()==1) {
//        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiWpa *connHidWifiWpa = new DlgConnHidWifiWpa(0, 0, this->parentWidget());
        connHidWifiWpa->show();
    } else if(ui->cbxSecurity->currentIndex()==2) {
//        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiWep *connHidWifiWep = new DlgConnHidWifiWep(0, this->parentWidget());
        connHidWifiWep->show();
    } else if(ui->cbxSecurity->currentIndex()==3) {
//        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiWep *connHidWifiWep = new DlgConnHidWifiWep(1, this->parentWidget());
        connHidWifiWep->show();
    } else if(ui->cbxSecurity->currentIndex()==4) {
//        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiLeap *connHidWifiLeap = new DlgConnHidWifiLeap(this->parentWidget());
        connHidWifiLeap->show();
    } else if(ui->cbxSecurity->currentIndex()==5) {
        if (WepOrWpa == 1) {
            ui->cbxSecurity->setCurrentIndex(5);
            WepOrWpa = 0;
        }
    } else {
        if (WepOrWpa == 0){
            ui->cbxSecurity->setCurrentIndex(6);
            WepOrWpa = 1;
        }
    }
}

void DlgConnHidWifiSecTls::changeDialogAuth()
{
    if(ui->cbxAuth->currentIndex()==0){
        qDebug()<<"it's not need to change dialog";
    } else if(ui->cbxAuth->currentIndex()==1) {
//        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiSecLeap *connHidWifiSecLeap = new DlgConnHidWifiSecLeap(WepOrWpa);
        connHidWifiSecLeap->show();
    } else if(ui->cbxAuth->currentIndex()==2) {
//        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiSecPwd *connHidWifiSecPwd = new DlgConnHidWifiSecPwd(WepOrWpa);
        connHidWifiSecPwd->show();
    } else if(ui->cbxAuth->currentIndex()==3) {
//        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiSecFast *connHidWifiSecFast = new DlgConnHidWifiSecFast(WepOrWpa);
        connHidWifiSecFast->show();
    } else if(ui->cbxAuth->currentIndex()==4) {
//        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiSecTunnelTLS *connHidWifiSecTuTls = new DlgConnHidWifiSecTunnelTLS(WepOrWpa);
        connHidWifiSecTuTls->show();
    } else {
//        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiSecPeap *connHidWifiSecPeap = new DlgConnHidWifiSecPeap(WepOrWpa);
        connHidWifiSecPeap->show();
    }
}

void DlgConnHidWifiSecTls::on_cbxCA_currentIndexChanged(const QString &arg1)
{
    if (ui->cbxCA->currentIndex() == 0){
        ui->leCaPwd->setText("");
        ui->lbCaPwd->setEnabled(false);
        ui->leCaPwd->setEnabled(false);
    }else{
        ui->leCaPwd->setText("");
        ui->lbCaPwd->setEnabled(true);
        ui->leCaPwd->setEnabled(true);
    }
}

void DlgConnHidWifiSecTls::on_cbxUserCertify_currentIndexChanged(const QString &arg1)
{
    if (ui->cbxUserCertify->currentIndex() == 0){
        ui->leUserCertifyPwd->setText("");
        ui->lbUserCertifyPwd->setEnabled(false);
        ui->leUserCertifyPwd->setEnabled(false);

        ui->lbUserPriKey->setEnabled(false);
        ui->cbxUserPriKey->setEnabled(false);
        ui->cbxUserPriKey->setCurrentIndex(0);

        ui->leUserKeyPwd->setText("");
        ui->lbUserKeyPwd->setEnabled(false);
        ui->leUserKeyPwd->setEnabled(false);
    }else{
        ui->leUserCertifyPwd->setText("");
        ui->lbUserCertifyPwd->setEnabled(true);
        ui->leUserCertifyPwd->setEnabled(true);

        ui->lbUserPriKey->setEnabled(true);
        ui->cbxUserPriKey->setEnabled(true);
        ui->cbxUserPriKey->setCurrentIndex(0);

        ui->leUserKeyPwd->setText("");
        ui->lbUserKeyPwd->setEnabled(false);
        ui->leUserKeyPwd->setEnabled(false);
    }
}

void DlgConnHidWifiSecTls::on_cbxUserPriKey_currentIndexChanged(const QString &arg1)
{
    if (ui->cbxUserPriKey->currentIndex() == 0){
        ui->leUserKeyPwd->setText("");
        ui->lbUserKeyPwd->setEnabled(false);
        ui->leUserKeyPwd->setEnabled(false);
    }else{
        ui->leUserKeyPwd->setText("");
        ui->lbUserKeyPwd->setEnabled(true);
        ui->leUserKeyPwd->setEnabled(true);
    }
}

void DlgConnHidWifiSecTls::on_btnCancel_clicked()
{
    this->close();
}

void DlgConnHidWifiSecTls::on_btnConnect_clicked()
{
    this->close();
}

void DlgConnHidWifiSecTls::on_checkBoxCA_stateChanged(int arg1)
{
    if (arg1 == 0) {
        ui->cbxCA->setCurrentIndex(0);
        ui->lbCA->setEnabled(true);
        ui->cbxCA->setEnabled(true);
    } else {
        ui->cbxCA->setCurrentIndex(0);
        ui->lbCA->setEnabled(false);
        ui->cbxCA->setEnabled(false);
    }
}

void DlgConnHidWifiSecTls::on_checkBoxPwd_stateChanged(int arg1)
{
    if (arg1 == 0) {
        ui->leCaPwd->setEchoMode(QLineEdit::Password);
    } else {
        ui->leCaPwd->setEchoMode(QLineEdit::Normal);
    }
}

void DlgConnHidWifiSecTls::on_checkBoxPwdSec_stateChanged(int arg1)
{
    if (arg1 == 0) {
        ui->leUserCertifyPwd->setEchoMode(QLineEdit::Password);
        ui->leUserKeyPwd->setEchoMode(QLineEdit::Password);
    } else {
        ui->leUserCertifyPwd->setEchoMode(QLineEdit::Normal);
        ui->leUserKeyPwd->setEchoMode(QLineEdit::Normal);
    }
}

void DlgConnHidWifiSecTls::on_leNetName_textEdited(const QString &arg1)
{
    if (ui->leNetName->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leIdentity->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leDomain->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else {
        if (ui->cbxCA->currentIndex() == 0){
            if (ui->cbxUserCertify->currentIndex() == 0){
                ui->btnConnect->setEnabled(true);
            }else {
                if (ui->leUserCertifyPwd->text() == ""){
                    ui->btnConnect->setEnabled(false);
                } else {
                    if (ui->cbxUserPriKey->currentIndex() == 0){
                        ui->btnConnect->setEnabled(true);
                    }else if (ui->leUserKeyPwd->text() == ""){
                        ui->btnConnect->setEnabled(false);
                    } else{
                        ui->btnConnect->setEnabled(true);
                    }
                }
            }
        } else {
            if (ui->leCaPwd->text() == ""){
                ui->btnConnect->setEnabled(false);
            } else {
                if (ui->cbxUserCertify->currentIndex() == 0){
                    ui->btnConnect->setEnabled(true);
                }else {
                    if (ui->leUserCertifyPwd->text() == ""){
                        ui->btnConnect->setEnabled(false);
                    } else {
                        if (ui->cbxUserPriKey->currentIndex() == 0){
                            ui->btnConnect->setEnabled(true);
                        }else if (ui->leUserKeyPwd->text() == ""){
                            ui->btnConnect->setEnabled(false);
                        } else{
                            ui->btnConnect->setEnabled(true);
                        }
                    }
                }
            }
        }
    }
}

void DlgConnHidWifiSecTls::on_leIdentity_textEdited(const QString &arg1)
{
    if (ui->leNetName->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leIdentity->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leDomain->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else {
        if (ui->cbxCA->currentIndex() == 0){
            if (ui->cbxUserCertify->currentIndex() == 0){
                ui->btnConnect->setEnabled(true);
            }else {
                if (ui->leUserCertifyPwd->text() == ""){
                    ui->btnConnect->setEnabled(false);
                } else {
                    if (ui->cbxUserPriKey->currentIndex() == 0){
                        ui->btnConnect->setEnabled(true);
                    }else if (ui->leUserKeyPwd->text() == ""){
                        ui->btnConnect->setEnabled(false);
                    } else{
                        ui->btnConnect->setEnabled(true);
                    }
                }
            }
        } else {
            if (ui->leCaPwd->text() == ""){
                ui->btnConnect->setEnabled(false);
            } else {
                if (ui->cbxUserCertify->currentIndex() == 0){
                    ui->btnConnect->setEnabled(true);
                }else {
                    if (ui->leUserCertifyPwd->text() == ""){
                        ui->btnConnect->setEnabled(false);
                    } else {
                        if (ui->cbxUserPriKey->currentIndex() == 0){
                            ui->btnConnect->setEnabled(true);
                        }else if (ui->leUserKeyPwd->text() == ""){
                            ui->btnConnect->setEnabled(false);
                        } else{
                            ui->btnConnect->setEnabled(true);
                        }
                    }
                }
            }
        }
    }
}

void DlgConnHidWifiSecTls::on_leDomain_textEdited(const QString &arg1)
{
    if (ui->leNetName->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leIdentity->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leDomain->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else {
        if (ui->cbxCA->currentIndex() == 0){
            if (ui->cbxUserCertify->currentIndex() == 0){
                ui->btnConnect->setEnabled(true);
            }else {
                if (ui->leUserCertifyPwd->text() == ""){
                    ui->btnConnect->setEnabled(false);
                } else {
                    if (ui->cbxUserPriKey->currentIndex() == 0){
                        ui->btnConnect->setEnabled(true);
                    }else if (ui->leUserKeyPwd->text() == ""){
                        ui->btnConnect->setEnabled(false);
                    } else{
                        ui->btnConnect->setEnabled(true);
                    }
                }
            }
        } else {
            if (ui->leCaPwd->text() == ""){
                ui->btnConnect->setEnabled(false);
            } else {
                if (ui->cbxUserCertify->currentIndex() == 0){
                    ui->btnConnect->setEnabled(true);
                }else {
                    if (ui->leUserCertifyPwd->text() == ""){
                        ui->btnConnect->setEnabled(false);
                    } else {
                        if (ui->cbxUserPriKey->currentIndex() == 0){
                            ui->btnConnect->setEnabled(true);
                        }else if (ui->leUserKeyPwd->text() == ""){
                            ui->btnConnect->setEnabled(false);
                        } else{
                            ui->btnConnect->setEnabled(true);
                        }
                    }
                }
            }
        }
    }
}

void DlgConnHidWifiSecTls::on_leCaPwd_textEdited(const QString &arg1)
{
    if (ui->leNetName->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leIdentity->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leDomain->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if(ui->leCaPwd->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else {
        if (ui->cbxUserCertify->currentIndex() == 0){
            ui->btnConnect->setEnabled(true);
        }else {
            if (ui->leUserCertifyPwd->text() == ""){
                ui->btnConnect->setEnabled(false);
            } else {
                if (ui->cbxUserPriKey->currentIndex() == 0){
                    ui->btnConnect->setEnabled(true);
                }else if (ui->leUserKeyPwd->text() == ""){
                    ui->btnConnect->setEnabled(false);
                } else{
                    ui->btnConnect->setEnabled(true);
                }
            }
        }
    }
}

void DlgConnHidWifiSecTls::on_leUserCertifyPwd_textEdited(const QString &arg1)
{
    if (ui->leNetName->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leIdentity->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leDomain->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else {
        if (ui->cbxCA->currentIndex() == 0){
            if (ui->leUserCertifyPwd->text() == ""){
                ui->btnConnect->setEnabled(false);
            }else{
                if (ui->cbxUserPriKey->currentIndex() == 0){
                    ui->btnConnect->setEnabled(true);
                }else if (ui->leUserKeyPwd->text() == ""){
                    ui->btnConnect->setEnabled(false);
                } else{
                    ui->btnConnect->setEnabled(true);
                }
            }
        } else {
            if (ui->leCaPwd->text() == ""){
                ui->btnConnect->setEnabled(false);
            }else{
                if (ui->leUserCertifyPwd->text() == ""){
                    ui->btnConnect->setEnabled(false);
                }else{
                    if (ui->cbxUserPriKey->currentIndex() == 0){
                        ui->btnConnect->setEnabled(true);
                    }else if (ui->leUserKeyPwd->text() == ""){
                        ui->btnConnect->setEnabled(false);
                    } else{
                        ui->btnConnect->setEnabled(true);
                    }
                }
            }
        }
    }
}

void DlgConnHidWifiSecTls::on_leUserKeyPwd_textEdited(const QString &arg1)
{
    if (ui->leNetName->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leIdentity->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else if (ui->leDomain->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else {
        if (ui->cbxCA->currentIndex() == 0){
            if (ui->leUserCertifyPwd->text() == ""){
                ui->btnConnect->setEnabled(false);
            }else if (ui->leUserKeyPwd->text() == ""){
                ui->btnConnect->setEnabled(false);
            } else{
                ui->btnConnect->setEnabled(true);
            }
        } else {
            if (ui->leCaPwd->text() == ""){
                ui->btnConnect->setEnabled(false);
            }else{
                if (ui->leUserCertifyPwd->text() == ""){
                    ui->btnConnect->setEnabled(false);
                }else if (ui->leUserKeyPwd->text() == ""){
                    ui->btnConnect->setEnabled(false);
                } else{
                    ui->btnConnect->setEnabled(true);
                }
            }
        }
    }
}

void DlgConnHidWifiSecTls::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
       opt.init(this);
       QPainter p(this);
       style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
       QWidget::paintEvent(event);
}
