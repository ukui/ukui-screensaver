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

#ifndef VERTICALVERIFICATIONWIDGET_H
#define VERTICALVERIFICATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <verificationwidget.h>

#include "common.h"

class VerticalVerificationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VerticalVerificationWidget(QWidget *parent = nullptr);
    ~VerticalVerificationWidget();

    void reloadQR();

Q_SIGNALS:
    void pageMessage(SwitchPage id, QList<QVariant> argvs);
private:
    QLabel *m_titleLB;
    QLabel *m_phonePromptMsgLB;
    WeChatAuthDialog *m_weChatWidget;
//public:
    PhoneAuthWidget *m_phoneAuthWidget;

protected:
    void paintEvent(QPaintEvent *);
};

#endif // VERTICALVERIFICATIONWIDGET_H
