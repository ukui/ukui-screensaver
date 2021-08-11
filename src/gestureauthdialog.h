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
#ifndef GESTUREAUTHDIALOG_H
#define GESTUREAUTHDIALOG_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QLabel>

class GestureAuthDialog : public QWidget
{
    Q_OBJECT
public:
    struct PosInCircle
    {
        int i;      //行
        int j;      //列
        bool isIn;  //是否在范围内
    };
    explicit GestureAuthDialog(QWidget *parent = nullptr);
    ~GestureAuthDialog();

    void setPassword(const QString password);
    QString getPassword();
    bool checkPassword();
protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void drawBg(QPainter *painter);
    void drawOutCircle(QPainter *painter);
    void drawTipLine(QPainter *painter);
    void drawInCircle(QPainter *painter);
    void drawJoinLine(QPainter *painter);
private:
    double distance(int x1, int y1, int x2, int y2);
    int posToNumber(int i, int j);
Q_SIGNALS:
    void correctPassword(bool isCorrect, QString password);
    void authenticateCompete(bool result);
public Q_SLOTS:
private:
    QColor m_bgColor;                   //背景色
    QColor m_outCircleColor;            //外圆颜色
    QColor m_tipLineColor;              //提示线颜色
    QColor m_circleRingColor;           //圆环颜色
    int m_margin;                       //外边距
    int m_radius;                       //圆相对半径
    PosInCircle m_posInCircle;          //判断点是否在圆内
    bool m_pressed;                     //鼠标按下
    bool m_showText;                    //是否显示文字

    QVector<QPoint> m_vecInputPoints;   //鼠标移动选点
    QPoint m_followPoint;               //鼠标跟随点
    QString m_password;                 //原密码
    QString m_inputPassword;            //鼠标移动选点转换为密码

    QPushButton *m_set_password;
    bool m_is_set_password;
    QPushButton *m_unlock;
    bool m_is_check_password;
    QLabel *m_chech_result;
};

#endif // GESTUREAUTHDIALOG_H
