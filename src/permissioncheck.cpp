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

#include "permissioncheck.h"
#include <QGridLayout>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include "common.h"

#include "wechatauthdialog.h"

VerificationWidget::VerificationWidget(QWidget *parent) : QWidget(parent)
{
    qDebug() << "PermissionCheck";
    resize(1000,355);
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    setLayout(mainLayout);

    WeChatAuthDialog *m_weChatWidget;
    PhoneAuthWidget *m_phoneAuthWidget;

    m_weChatWidget = new WeChatAuthDialog(1,this);
    mainLayout->addWidget(m_weChatWidget, 1, Qt::AlignLeft);

    mainLayout->addSpacing(400);

    m_phoneAuthWidget = new PhoneAuthWidget(this);
    mainLayout->addWidget(m_phoneAuthWidget, 1, Qt::AlignRight);

    connect(m_phoneAuthWidget, &PhoneAuthWidget::pageMessage, this, [=](SwitchPage s, QList<QVariant> list){
        Q_EMIT pageMessage(s, list);
    });
}

void VerificationWidget::paintEvent(QPaintEvent *)
{
    int x1 = width() / 2;
    int y1 = height() / 4;
    int x2 = x1;
    int y2 = height() / 3 * 2 + y1;
    QLinearGradient linearGra(QPoint(x1, y1), QPoint(x2, y2));
    linearGra.setColorAt(0, QColor(238, 238, 238, 0));
    linearGra.setColorAt(0.5, QColor(255, 255, 255, 138));
    linearGra.setColorAt(1, QColor(216, 216, 216, 0));

    QPainter painter(this);
    QBrush brush(linearGra);
    painter.setPen(Qt::transparent);
    painter.setBrush(brush);
    painter.drawRoundedRect(QRect(x1, y1, 4, height() / 3 * 2), 16, 16);
}

InputInfos::InputInfos(QWidget *parent):
    QWidget(parent)
{
    initUI();
    initConnect();
    setQSS();
}

void InputInfos::initUI()
{
    setFixedWidth(316);
    QGridLayout *mainLayout = new QGridLayout(this);
    setLayout(mainLayout);
    mainLayout->setColumnStretch(1, 0);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->setColumnStretch(3, 0);

    m_pPhoneIconLB = new QLabel(this);
    m_pPhoneIconLB->setPixmap(QPixmap(":/images/icon-phone.png"));
    mainLayout->addWidget(m_pPhoneIconLB, 0, 1, 1, 1, Qt::AlignLeft);

    m_pPhoneLE = new QLineEdit(this);
    m_pPhoneLE->setPlaceholderText("手机号");
    m_pPhoneLE->setProperty("class", "InputLine");
    m_pPhoneLE->setValidator(0);
    mainLayout->addWidget(m_pPhoneLE, 0, 2, 1, 2, Qt::AlignLeft);

    m_pVerCodeIconLB = new QLabel(this);
    m_pVerCodeIconLB->setPixmap(QPixmap(":/images/icon-sms.png"));
    mainLayout->addWidget(m_pVerCodeIconLB, 1, 1, 1, 1, Qt::AlignLeft);

    m_pVerCodeLE = new QLineEdit(this);
    m_pVerCodeLE->setPlaceholderText("短信验证码");
    m_pVerCodeLE->setProperty("class", "InputLine");
    m_pVerCodeLE->setValidator(0);
    mainLayout->addWidget(m_pVerCodeLE, 1, 2, 1, 1, Qt::AlignLeft);

    m_pGetVerCodeBT = new QPushButton(this);
    m_pGetVerCodeBT->setText("获取验证码");
    m_pGetVerCodeBT->setEnabled(false);
    m_pGetVerCodeBT->setFocusPolicy(Qt::NoFocus);
    m_pGetVerCodeBT->setProperty("class", "GetCodeBT");
    m_pGetVerCodeBT->setCursor(QCursor(Qt::PointingHandCursor));
    mainLayout->addWidget(m_pGetVerCodeBT, 1, 3, 1, 1, Qt::AlignLeft);

    m_pNextGetVerCodeQTimer = new QTimer(this);

    m_curInputState = InputState::InputWaiting;
}

void InputInfos::initConnect()
{
    connect(m_pPhoneLE, &QLineEdit::textChanged, this, &InputInfos::onPhoneTextChanged);

    connect(m_pVerCodeLE, &QLineEdit::textChanged, this, &InputInfos::onVerCodeTextChanged);

    connect(m_pNextGetVerCodeQTimer, &QTimer::timeout, this, &InputInfos::onNextGetVerCodeTimer);

    connect(m_pGetVerCodeBT, &QPushButton::clicked, this, &InputInfos::onGetVerCode);
}

void InputInfos::setQSS()
{
    QString style_sheet = ".InputLine{"
                          "padding:10px 2px 10px 2px;"
                          "}"
                          ".GetCodeBT{"
                          "background:rgba(0,0,0,0);"
                          "color:rgba(38,38,38,115);"
                          "font-size:16px;"
                          "}";
    setStyleSheet(style_sheet);
    adjustSize();
}

void InputInfos::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(rect(), 16, 16);
    painter.setPen(QColor(226, 226, 226));
    painter.drawLine(QLineF(0, m_pPhoneIconLB->y() + m_pPhoneIconLB->height() + 1,
                            width(), m_pPhoneIconLB->y() + m_pPhoneIconLB->height() + 1));
}

void InputInfos::onPhoneTextChanged(const QString &text)
{
    if(text.length() == 11){
        m_pGetVerCodeBT->setEnabled(true);
        m_pGetVerCodeBT->setFocusPolicy(Qt::NoFocus);
        m_pGetVerCodeBT->setStyleSheet("color:#2FB3E8;");

        if(m_pVerCodeLE->text().length() == 6){
            Q_EMIT InputStateChanged(InputState::InputFinish);
            m_curInputState = InputState::InputFinish;
        }
    }
    else
    {
        m_pGetVerCodeBT->setEnabled(false);
        m_pGetVerCodeBT->setStyleSheet("color:rgba(38, 38, 38, 115);");
        if(InputState::InputFinish == m_curInputState)
        {
            m_curInputState = InputState::InputWaiting;
            Q_EMIT InputStateChanged(InputState::InputWaiting);
        }
    }
}

void InputInfos::onVerCodeTextChanged(const QString &text)
{
    if(text.length() == 6 && m_pPhoneLE->text().length() == 11)
    {
        Q_EMIT InputStateChanged(InputState::InputFinish);
        m_curInputState = InputState::InputFinish;
    }
    else if(m_curInputState == InputState::InputFinish)
    {
        Q_EMIT InputStateChanged(InputState::InputWaiting);
        m_curInputState = InputState::InputWaiting;
    }
}

void InputInfos::clearInfos()
{
    m_pPhoneLE->clear();
    m_pVerCodeLE->clear();
    if(m_curInputState == InputState::InputFinish)
    {
        m_curInputState = InputState::InputWaiting;
        Q_EMIT InputStateChanged(m_curInputState);
    }
}

QString InputInfos::getVerificationCode()
{
    return m_pVerCodeLE->text();
}

QString InputInfos::getPhoneNumer()
{
    return m_pPhoneLE->text();
}

void InputInfos::onGetVerCode()
{
    m_pGetVerCodeBT->setEnabled(false);
    m_pGetVerCodeBT->setStyleSheet("color:rgba(38, 38, 38, 115);");
    m_nextGetVerCodeTime = 60;
    m_pGetVerCodeBT->setText("重新获取(60s)");
    m_pNextGetVerCodeQTimer->start(1000);
}


void InputInfos::onNextGetVerCodeTimer()
{
    --m_nextGetVerCodeTime;
    m_pGetVerCodeBT->setText("重新获取(" + QString::number(m_nextGetVerCodeTime) + "s)");
    if(m_nextGetVerCodeTime == 0)
    {
        m_pNextGetVerCodeQTimer->stop();
        m_pGetVerCodeBT->setEnabled(true);
        m_pGetVerCodeBT->setFocusPolicy(Qt::NoFocus);
        m_pGetVerCodeBT->setText("获取验证码");
        m_pGetVerCodeBT->setStyleSheet("color:#2FB3E8;");
        return;
    }

}

PhoneAuthWidget::PhoneAuthWidget(QWidget *parent)
{
    initUI();
    initConnect();
    setQSS();
}

void PhoneAuthWidget::initUI()
{
    QVBoxLayout *phoLayout = new QVBoxLayout(this);
    phoLayout->setMargin(0);

    m_pPhoTitleLB = new QLabel(this);
    m_pPhoTitleLB->setText(tr("Verification by phoneNum"));
    m_pPhoTitleLB->setProperty("class", "titleLB");
    phoLayout->addWidget(m_pPhoTitleLB, 0, Qt::AlignHCenter);

    m_pPhoPromptMsgLB = new QLabel(this);
    m_pPhoPromptMsgLB->setText(tr("「 Use bound Phone number to verification 」"));
    m_pPhoPromptMsgLB->setProperty("class", "PromptText");
    phoLayout->addWidget(m_pPhoPromptMsgLB, 0, Qt::AlignHCenter);

    m_pPhoErrorMsgLB = new QLabel(this);
    m_pPhoErrorMsgCloneLB = new QLabel(this);
    m_pPhoErrorMsgLB->setProperty("class", "ErrorMsg");
    m_pPhoErrorMsgCloneLB->setProperty("class", "ErrorMsg");
    hidePhoneErrorMsg();
    phoLayout->addWidget(m_pPhoErrorMsgLB, 0, Qt::AlignHCenter);
    phoLayout->addWidget(m_pPhoErrorMsgCloneLB, 0, Qt::AlignHCenter);


    m_pPhoInputInfos = new InputInfos(this);
    phoLayout->addWidget(m_pPhoInputInfos, 0, Qt::AlignHCenter);

    m_pPhoSubmitBT = new QPushButton(this);
    m_pPhoSubmitBT->setFocusPolicy(Qt::NoFocus);
    m_pPhoSubmitBT->setText(tr("commit"));
    m_pPhoSubmitBT->setProperty("class", "BindBT");
    m_pPhoSubmitBT->setCursor(QCursor(Qt::PointingHandCursor));
    m_pPhoSubmitBT->setEnabled(false);
    phoLayout->addSpacing(8);
    phoLayout->addWidget(m_pPhoSubmitBT, 0, Qt::AlignHCenter);
    phoLayout->addStretch(1);
}

void PhoneAuthWidget::initConnect()
{
    connect(m_pPhoInputInfos, &InputInfos::InputStateChanged, this, &PhoneAuthWidget::onInputStateChanged);

    connect(m_pPhoSubmitBT, &QPushButton::clicked, this, &PhoneAuthWidget::onSubmitBTClick);

    connect(m_pPhoInputInfos, &InputInfos::getVerCode, this, &PhoneAuthWidget::onGetVerCode);
}

QSize PhoneAuthWidget::sizeHint() const{
    return QWidget::sizeHint();
}

void PhoneAuthWidget::hidePhoneErrorMsg()
{
    m_pPhoErrorMsgCloneLB->setFixedHeight(m_pPhoErrorMsgLB->height());
    m_pPhoErrorMsgCloneLB->show();
    m_pPhoErrorMsgLB->hide();
}

void PhoneAuthWidget::showPhoneErrorMsg()
{
    m_pPhoErrorMsgLB->show();
    m_pPhoErrorMsgCloneLB->hide();
}

void PhoneAuthWidget::onInputStateChanged(InputInfos::InputState input_state)
{
    if(input_state == InputInfos::InputState::InputFinish)
    {
        m_pPhoSubmitBT->setEnabled(true);
        m_pPhoSubmitBT->setFocusPolicy(Qt::NoFocus);
        m_pPhoSubmitBT->setStyleSheet("background:#2FB3E8;");
    }
    else
    {
        m_pPhoSubmitBT->setEnabled(false);
        m_pPhoSubmitBT->setStyleSheet("background:rgba(255,255,255,115);");
    }
}


void PhoneAuthWidget::onSubmitBTClick()
{
    QString tel = m_pPhoInputInfos->getPhoneNumer();
    QString verCode = m_pPhoInputInfos->getVerificationCode();
    if (true)
    {
        //验证成功
        qDebug() << "-----------onSubmitBTClick";
        Q_EMIT pageMessage(SwitchPage::SwitchToResetPWD, QList<QVariant>());
    }
//    DBusMsgCode msgCode = SSOP->m_pRetrievePwdInterface->CheckVerificationCode(tel, verCode);
//    switch (msgCode) {
//    case DBusMsgCode::No_Error:
//        emit pageMessage(PageMessage::JumpToNewPswSet, QList<QVariant>());
//        break;
//        // TODO 更新提交后的错误状态
//    default:
//        break;
//    }
}

void PhoneAuthWidget::onGetVerCode()
{
//    DBusMsgCode msgCode = SSOP->m_pRetrievePwdInterface->GetVerificationCode(Cfg->getUsername(), m_pPhoInputInfos->getPhoneNumer());
//    switch (msgCode) {
//    case DBusMsgCode::No_Error:
//        // 验证码获取无错误
//        break;
//        // TODO 更新错误状态
//    default:
//        break;
//    }
}

void PhoneAuthWidget::onQRCodeStateChanged(QString username, QString password, int nState)
{
    QRCodeSwepState state = static_cast<QRCodeSwepState>(nState);
    switch (state) {
    case QRCodeSwepState::WaitingSwep:
        qDebug() << "info: [QRCodePhoneAuthWidget][onQRCodeStateChanged]: waiting user swep qrcode!";
        break;
    case QRCodeSwepState::HaveSwep:
        qDebug() << "info: [QRCodePhoneAuthWidget][onQRCodeStateChanged]: user has swep code!";
        break;
    case QRCodeSwepState::CancelSwep:
        qDebug() << "info: [QRCodePhoneAuthWidget][onQRCodeStateChanged]: user cancel swep code!";
        break;
    case QRCodeSwepState::ConfirmSuccess:
    {
        qDebug() << "info: [QRCodePhoneAuthWidget][onQRCodeStateChanged]: user = [" << username << "] confirm success!";
//        if(username != Cfg->getUsername())
//        {
//            resetQRCService();
//            // TODO 更新错误信息
//        }
//        else
//            emit pageMessage(PageMessage::JumpToNewPswSet, QList<QVariant>());
//        break;
    }
    case QRCodeSwepState::QRCodeInvalid:
    {
        qDebug() << "info: [QRCodePhoneAuthWidget][onQRCodeStateChanged]: qrcode invalid!";
//        resetQRCService();
        break;
    }
    default:
        break;
    }
}

void PhoneAuthWidget::setQSS()
{
    QString style_sheet =".PromptText{"
                        "font-size:24px;"
                        "color: rgba(255,255,255,192);"
                        "}"
                        ".BindBT{"
                        "background:rgba(255,255,255,115);"
                        "width:316px;"
                        "height:64px;"
                        "border-radius:16px;"
                        "}"
                        ".ErrorMsg{"
                        "color:#FD625E;"
                        "font-size:14px;"
                        "}"
                        ".titleLB{"
                         "color:#FFFFFF;"
                         "font-size:32px;"
                         "}";
    setStyleSheet(style_sheet);
}
