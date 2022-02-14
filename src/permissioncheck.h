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

#ifndef PERMISSIONCHECK_H
#define PERMISSIONCHECK_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "wechatauthdialog.h"
#include "common.h"

class InputInfos;
class PhoneAuthWidget;

class InputInfos : public QWidget
{
    Q_OBJECT
public:
    enum InputState{
        InputWaiting
        , InputFinish
    };
    InputInfos(QWidget *parent = nullptr);
    QString getPhoneNumer();
    QString getVerificationCode();
    void clearInfos();

Q_SIGNALS:
    void getVerCode();
    void InputStateChanged(InputState input_state);

protected:
    void paintEvent(QPaintEvent *);

private Q_SLOTS:
    void onPhoneTextChanged(const QString &text);
    void onVerCodeTextChanged(const QString &text);
    void onGetVerCode();
    void onNextGetVerCodeTimer();

private:
    void initUI();
    void initConnect();
    void setQSS();

private:
    QLabel *m_pPhoneIconLB;
    QLineEdit *m_pPhoneLE;
    QLabel *m_pVerCodeIconLB;
    QLineEdit *m_pVerCodeLE;
    QPushButton *m_pGetVerCodeBT;
    QTimer *m_pNextGetVerCodeQTimer;
    int m_nextGetVerCodeTime;

    InputState m_curInputState;
};

class VerificationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VerificationWidget(QWidget *parent = nullptr);

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

class PhoneAuthWidget : public QWidget
{
    Q_OBJECT
public:
    PhoneAuthWidget(QWidget* parent = nullptr);
    QSize sizeHint() const;

private:
    void initUI();
    void initConnect();
    void setQSS();

    void showPhoneErrorMsg();
    void hidePhoneErrorMsg();
Q_SIGNALS:
    void pageMessage(SwitchPage id, QList<QVariant> argvs);
private Q_SLOTS:
    void onInputStateChanged(InputInfos::InputState input_state);
    void onSubmitBTClick();
    void onGetVerCode();
    void onQRCodeStateChanged(QString username, QString password, int nState);

private:
    // 手机号验证
    QLabel *m_pPhoTitleLB;
    QLabel *m_pPhoPromptMsgLB;
    QLabel *m_pPhoErrorMsgLB;
    QLabel *m_pPhoErrorMsgCloneLB; // 控制格式
    QPushButton *m_pPhoSubmitBT;
    InputInfos *m_pPhoInputInfos;

};
#endif // PERMISSIONCHECK_H
