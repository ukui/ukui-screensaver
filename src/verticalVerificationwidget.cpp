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

#include "verticalVerificationwidget.h"
#include <QGridLayout>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRegExpValidator>
#include <QSpacerItem>
#include <QSizePolicy>

#include "common.h"

#include "wechatauthdialog.h"
#include "eduplatforminterface.h"
#include "accountsinterface.h"

/**
 * 忘记密码后，校验页面：分为二维码校验和手机号校验
 */

VerticalVerificationWidget::VerticalVerificationWidget(QWidget *parent) : QWidget(parent)
{
    qDebug() << "PermissionCheck";
    resize(600,1000);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    m_weChatWidget = new WeChatAuthDialog(1,this);
    mainLayout->addWidget(m_weChatWidget, 1, Qt::AlignCenter);

    mainLayout->addSpacerItem(new QSpacerItem(0, 250, QSizePolicy::Fixed, QSizePolicy::Expanding));

    m_phoneAuthWidget = new PhoneAuthWidget(this);
    mainLayout->addWidget(m_phoneAuthWidget, 1, Qt::AlignCenter);

    connect(m_phoneAuthWidget, &PhoneAuthWidget::pageMessage, this, [=](SwitchPage s, QList<QVariant> list){
        Q_EMIT pageMessage(s, list);
    });

    connect(m_weChatWidget, &WeChatAuthDialog::qRStatusChange, this, [=] (QString loginname, QString loginpwd, int curstatus){
        //QString loginname, QString loginpwd, int curstatus
        if (loginname.isEmpty())
        {
            // Todo？正在登陆？？？？
            qDebug() << "wechar name is null";
        } else if(getenv("USER") == loginname)
        {
            Q_EMIT pageMessage(SwitchPage::SwitchToResetPWD, QList<QVariant>());;
        } else {
            qWarning() << "[error] <VerificationWidget> Username does not match wechat<" << loginname << ">" << " local<" << getenv("USER")<<">";
            m_weChatWidget->showErrorMessage(tr("Please scan by bound WeChat"));
            m_weChatWidget->onReset();
        }
    });
}

VerticalVerificationWidget::~VerticalVerificationWidget()
{
    m_phoneAuthWidget->close();
    m_weChatWidget->close();
}

void VerticalVerificationWidget::paintEvent(QPaintEvent *)
{
    int x1 = width() / 3;
    int y1 = height() / 2;
    int x2 = width() / 3 * 2 ;
    int y2 = y1;
    QLinearGradient linearGra(QPoint(x1, y1), QPoint(x2, y2));
    linearGra.setColorAt(0, QColor(238, 238, 238, 0));
    linearGra.setColorAt(0.5, QColor(255, 255, 255, 138));
    linearGra.setColorAt(1, QColor(216, 216, 216, 0));

    QPainter painter(this);
    QBrush brush(linearGra);
    painter.setPen(Qt::transparent);
    painter.setBrush(brush);
    painter.drawRoundedRect(QRect(x1, y1,  width() / 3 * 2, 4), 16, 16);
}

void VerticalVerificationWidget::reloadQR()
{
    m_weChatWidget->reloadQR();
}


