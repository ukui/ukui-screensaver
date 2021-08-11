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
#ifndef DIGITALAUTHDIALOG_H
#define DIGITALAUTHDIALOG_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QLabel>
#include "auth.h"
#include "digitalkeyboard.h"
#include <QTimer>

class Auth;
class MyLabel;

class DigitalAuthDialog : public QWidget
{
    Q_OBJECT
public:
    struct PosInCircle
    {
        int i;      //行
        int j;      //列
        bool isIn;  //是否在范围内
    };

    enum LoginType{
        AUTHENTICATE = 0,
        RESET,
        SECONDCONFIRMATION,
    };

    explicit DigitalAuthDialog(QWidget *parent = nullptr);
    ~DigitalAuthDialog();

    void setPassword(const QString password);
    QString getPassword();
    bool checkPassword();
    void reset();
    void showResetPasswordPage();
    int getStatus();
    void showErrorMessage(QString message);
//    void RecieveKey(int key);
protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *evert) override;

    void drawBg(QPainter *painter);
    void drawOutCircle(QPainter *painter);
    void drawFilledCircle(QPainter *painter);
    void drawTipLine(QPainter *painter);
    void drawInCircle(QPainter *painter);
    void drawJoinLine(QPainter *painter);
    void drawStatusCircle(QPainter *painter);
    void drawHoverCircle(QPainter *painter);
private:
    /**
     * 圆形按钮，测量距离中心的距离
     * @brief distance
     * @param x1
     * @param y1
     * @param x2 圆心x坐标
     * @param y2 圆心y坐标
     * @return
     */
    double distance(int x1, int y1, int x2, int y2);

    bool isInButtonArea(int x1, int y1, int x2, int y2);
    int posToNumber(int i, int j);

//    void onShowMessage(const QString &message);
    void cleanMessage();

    void startWaiting();
    void stopWaiting();
Q_SIGNALS:
    void correctPassword(bool isCorrect, QString password);
    void authenticateCompete(bool result);
    void requestPasswordReset();
    void setPinCode(QString pinCode);
    void switchToReset(bool isReset);
private Q_SLOTS:
    void onShowMessage(const QString &message, Auth::MessageType type);
    void onShowPrompt(const QString &prompt, Auth::PromptType type);
    void onAuthComplete();
public Q_SLOTS:
    void startAuth();
    void stopAuth();
    void onNumerPress(int btn_id);
private:
    QColor m_bgColor;                   //背景色
    QColor m_outCircleColor;            //外圆颜色
    QColor m_tipLineColor;              //提示线颜色
    QColor m_circleRingColor;           //圆环颜色
    QColor m_circleColor;               //实心圆颜色
    int m_margin;                       //外边距
    int m_radius;                       //圆相对半径
    int m_buttonRadius;                //按钮圆角半径
    int m_buttonWidth;                 //按钮宽度
    int m_buttonHeight;                //按钮高度
    int m_buttonStep;                  //按钮间距
    PosInCircle m_posInCircle;          //判断点是否在圆内
    bool m_pressed;                     //鼠标按下
    bool m_showText;                    //是否显示文字

    QVector<QPoint> m_vecInputPoints;   //鼠标移动选点
    QPoint m_followPoint;               //鼠标跟随点
    QString m_password;                 //原密码
    QString m_inputPassword;            //鼠标移动选点转换为密码

    //label
    QLabel *m_title;
    MyLabel *m_labelReset;

    //密码状态更新布局
    int m_statusRadius;

    //认证中?
    bool m_isAuthing;

    //数字键盘
    DigitalKeyBoard *m_digitalKeyBoard;

    /**
     * 二次认证相关成员变量
     */
    int m_loginType = LoginType::AUTHENTICATE;
    QString m_resetPassword;
    QLabel *m_resetOrAuthFailure;

    QLabel *m_authMessage;

    //pam auth
    Auth                *m_auth;

//    QTimer              *m_timer;
};

class MyLabel : public QLabel{
    Q_OBJECT
public:
    MyLabel(const QString & text, QWidget *parent);
    MyLabel(QWidget *parent);

Q_SIGNALS:
    void onClick();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // DIGITALAUTHDIALOG_H
