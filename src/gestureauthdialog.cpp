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
#include "gestureauthdialog.h"

#include <QDebug>
#include <cmath>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

int gestureScale = 1;

/**
 * @author zhaikangning
 * @date 2020/09/15
 * 手势密码页
 * 功能未完善 未对接pam
 * 未来进化  所有登录均继承认证父类 该父类含有密码校验等基本功能
 */

GestureAuthDialog::GestureAuthDialog(QWidget *parent) : QWidget(parent),
    m_bgColor("#000000"),
    m_outCircleColor("#fff333"),
    m_tipLineColor("#ff0000"),
    m_circleRingColor("#ddffff"),
    m_radius(20),
    m_margin(5),
    m_pressed(false),
    m_showText(true),
    m_inputPassword(""),
    m_is_set_password(false),
    m_is_check_password(false)
{
    m_posInCircle.isIn = false;
    qDebug() <<"-----------init width=" << width() << ",height=" << height();

    m_set_password = new QPushButton(this);
    m_set_password->setGeometry(0,0,m_set_password->width(),m_set_password->height());
    m_set_password->setText("设置密码");
    connect(m_set_password, &QPushButton::clicked,this, [=] {
        m_chech_result->setText("请输入密码");
        m_is_set_password = true;
    });


    m_unlock = new QPushButton(this);
    m_unlock->setText("解锁");
    m_unlock->setGeometry(0,m_set_password->height() + 2,m_unlock->width(),m_unlock->height());
    connect(m_unlock,&QPushButton::clicked, this, [=] {
        m_chech_result->setText("密码校验");
        m_is_check_password = true;
    });

    m_chech_result = new QLabel("校验结果", this);
    m_chech_result->setStyleSheet("color:#ffeeff;");
    m_chech_result->setGeometry(0,m_set_password->height() + m_unlock->height() +2, m_chech_result->width(), m_chech_result->height());

    resize(640,480);
}

GestureAuthDialog::~GestureAuthDialog()
{

}

void GestureAuthDialog::paintEvent(QPaintEvent *event)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);
    qDebug() <<"-----------width=" << width << ",height=" << height << ",side=" << side;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width / 2, height / 2);
    painter.scale(side / 200, side / 200);
    gestureScale = side/200;

    drawBg(&painter);
    drawOutCircle(&painter);
    drawTipLine(&painter);
    drawInCircle(&painter);
    drawJoinLine(&painter);
}
void GestureAuthDialog::drawBg(QPainter *painter)
{
    painter->save();

    int width = this->width();
    int height = this->height();

    painter->setPen(Qt::NoPen);
    m_bgColor.setAlpha(100);
    painter->setBrush(m_bgColor);
    painter->drawRect(-width / 2, -height / 2, width, height);

    painter->restore();
}

void GestureAuthDialog::drawOutCircle(QPainter *painter)
{
    painter->save();
    QPen pen(m_outCircleColor, 2);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int x = -5 * m_radius + j * 4 * m_radius;
            int y = -5 * m_radius + i * 4 * m_radius;
            painter->drawEllipse(x, y, m_radius * 2, m_radius * 2);
//qDebug() << "-------------x,y=" << x << ","<< y;
            if (m_showText)
            {
                int number = posToNumber(i, j);
                QString text = QString::number(number);
                x = -4 * m_radius + j * 4 * m_radius;
                y = -4 * m_radius + i * 4 * m_radius;
                int textWidth = fontMetrics().width(text);
                int textHeight = fontMetrics().height();
                painter->drawText(x - textWidth / 2, y + textWidth / 2, text);
            }
        }
    }

    painter->restore();
}

void GestureAuthDialog::drawTipLine(QPainter *painter)
{
    painter->save();

    int offset = 3;

    if (m_posInCircle.isIn)
    {
        if (!m_pressed)
        {
            painter->setPen(m_tipLineColor);
            painter->setBrush(Qt::NoBrush);

            int x = -m_radius * 5 + m_posInCircle.j * 4 * m_radius - offset;
            int y = -m_radius * 5 + m_posInCircle.i * 4 * m_radius - offset;
            int w = (m_radius + offset) * 2;
            int h = (m_radius + offset) * 2;
            QRect rect(x, y, w, h);
            painter->drawEllipse(rect);
        }
    }

    painter->restore();
}

void GestureAuthDialog::drawInCircle(QPainter *painter)
{
    painter->save();

    if (m_pressed)
    {
        int offset1 = 1;//圆环偏移量
        int offset2 = 3;//内圆偏移量

        if (m_posInCircle.isIn)
        {
            //绘制圆环
            painter->setPen(m_circleRingColor);
            painter->setBrush(m_circleRingColor);
            int x = -m_radius * 5 + m_posInCircle.j * 4 * m_radius + offset1;
            int y = -m_radius * 5 + m_posInCircle.i * 4 * m_radius + offset1;
            int w = (m_radius - offset1) * 2;
            int h = (m_radius - offset1) * 2;
            QRect rect(x, y, w, h);
            painter->drawEllipse(rect);

            painter->setPen(m_outCircleColor);
            painter->setBrush(m_outCircleColor);
            x = -m_radius * 5 + m_posInCircle.j * 4 * m_radius + offset2;
            y = -m_radius * 5 + m_posInCircle.i * 4 * m_radius + offset2;
            w = (m_radius - offset2) * 2;
            h = (m_radius - offset2) * 2;
            rect = QRect(x, y, w, h);
            painter->drawEllipse(rect);

            x = -m_radius * 4 + m_posInCircle.j * 4 * m_radius;
            y = -m_radius * 4 + m_posInCircle.i * 4 * m_radius;

            if (!m_vecInputPoints.contains(QPoint(x, y)))
            {
                m_vecInputPoints.push_back(QPoint(x, y));
                int number = posToNumber(m_posInCircle.i, m_posInCircle.j);
                m_inputPassword += QString::number(number);
            }
        }
    }

    painter->restore();
}

void GestureAuthDialog::drawJoinLine(QPainter *painter)
{
    painter->save();

    QPen pen(m_outCircleColor, 3);
    painter->setPen(pen);
    //绘制连接线段
    for (int i = 0; i < m_vecInputPoints.count() - 1; i++)
    {
        painter->drawLine(m_vecInputPoints[i], m_vecInputPoints[i + 1]);
    }

    int offset1 = 1;//圆环偏移量
    int offset2 = 3;//内圆偏移量
    //绘制连接圆
    for (int i = 0; i < m_vecInputPoints.count(); i++)
    {
        int x = m_vecInputPoints[i].x() - m_radius + offset1;
        int y = m_vecInputPoints[i].y() - m_radius + offset1;
        int w = 2 * (m_radius - offset1);
        int h = 2 * (m_radius - offset1);
        painter->setPen(m_circleRingColor);
        painter->setBrush(m_circleRingColor);
        painter->drawEllipse(x, y, w, h);

        x = m_vecInputPoints[i].x() - m_radius + offset2;
        y = m_vecInputPoints[i].y() - m_radius + offset2;
        w = 2 * (m_radius - offset2);
        h = 2 * (m_radius - offset2);
        painter->setPen(m_outCircleColor);
        painter->setBrush(m_outCircleColor);
        painter->drawEllipse(x, y, w, h);
    }
    //绘制跟随线
    if (m_pressed && m_vecInputPoints.count() < 9)
    {
        int width = this->width();
        int height = this->height();
        int side = qMin(width, height);
        int gestureScale = side / 200;
        int x = (m_followPoint.x() - width / 2) / gestureScale;
        int y = (m_followPoint.y() - height / 2) / gestureScale;
        if (m_vecInputPoints.count() > 0)
        {
            painter->setPen(pen);
            painter->drawLine(m_vecInputPoints.last(), QPoint(x, y));
        }
    }

    painter->restore();
}

void GestureAuthDialog::mouseMoveEvent(QMouseEvent *event)
{
    //nothing
    qDebug() << "--------mouseMoveEvent(" << event->x() << "," << event->y() << ")";
    //    qDebug() << "-------------gestureScale=" << gestureScale;
        int x = (event->x() - this->width()/2) / gestureScale;
        int y = (event->y() - this->height()/2) / gestureScale;
    //    qDebug() << "--------mousePressEvent转化后 x,y(" << x << "," << y << ")";
        int jMax = 0;
        if (x >=0)
            jMax = 1;

        for (int i = 0; i < 3; i++)
        {
            for (int j = jMax; j < 3; j++)
            {

                int x1 = -4 * m_radius + j * 4 * m_radius;
                int y1 = -4 * m_radius + i * 4 * m_radius;
    //            qDebug() << "--------每个圆心的位置 x1,y1(" << x1 << "," << y1 << ")";
                if (distance(x , y, x1, y1) <= m_radius)
                {
                    QString numStr = QString::number(posToNumber(i,j), 10);
                   if (m_inputPassword.contains(numStr))
                   {
                       qWarning() << "已选过该区域";
                       return;
                   }
                   m_inputPassword.append(numStr);

                    m_posInCircle.i = i;
                    m_posInCircle.j = j;
//                    m_posInCircle.isIn = true;
                    qDebug() << "---------------选中了"<< posToNumber(i,j);


                    m_vecInputPoints.append(QPoint(x1, y1));   //鼠标移动选点
                    m_followPoint = QPoint(x, y);
                    repaint();
                }
            }
        }
}

void GestureAuthDialog::mousePressEvent(QMouseEvent *event)
{
    //nothing
    qDebug() << "--------mousePressEvent(" << event->x() << "," << event->y() << ")";
//    qDebug() << "-------------gestureScale=" << gestureScale;
    int x = (event->x() - this->width()/2) / gestureScale;
    int y = (event->y() - this->height()/2) / gestureScale;
//    qDebug() << "--------mousePressEvent转化后 x,y(" << x << "," << y << ")";

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int x1 = -4 * m_radius + j * 4 * m_radius;
            int y1 = -4 * m_radius + i * 4 * m_radius;
//            qDebug() << "--------每个圆心的位置 x1,y1(" << x1 << "," << y1 << ")";
            if (distance(x , y, x1, y1) <= m_radius)
            {
                m_posInCircle.i = i;
                m_posInCircle.j = j;
                m_posInCircle.isIn = true;

                m_vecInputPoints.append(QPoint(x1, y1));   //鼠标移动选点
                m_followPoint = QPoint(x1, y1);
                qDebug() << "---------------选中了"<< posToNumber(i,j);
                repaint();
            }
        }
    }
}

void GestureAuthDialog::mouseReleaseEvent(QMouseEvent *event)
{
    //nothing
    qDebug() << "--------mouseReleaseEvent(" << event->x() << "," << event->y() << ")";
    m_posInCircle.isIn = false;
    if (m_is_set_password && !m_inputPassword.isEmpty())
    {
        m_password = m_inputPassword;
        m_chech_result->setText("密码设置成功");
        qDebug() << "---------密码:" << m_password;
        m_is_set_password = false;
    }

    if (m_is_check_password) {
        if (m_inputPassword == m_password)
        {
            m_chech_result->setText("密码正确");
            qDebug() << "---------密码:" << m_password << "  输入的是：" << m_inputPassword;
            emit authenticateCompete(true);
//            m_is_check_password = false;
        } else {
            m_chech_result->setText("密码错误");
            qDebug() << "---------密码:" << m_password << "  输入的是：" << m_inputPassword;
        }
    }
    m_vecInputPoints.clear();
    m_inputPassword.clear();
    repaint();
}

double GestureAuthDialog::distance(int x1, int y1, int x2, int y2)
{
    //nothing
    double x = x1 - x2;
    double y = y1 - y2;
    return sqrt(x*x + y*y);
}

int GestureAuthDialog::posToNumber(int i, int j)
{
    //nothing
    return ((3*i) + j +1);
}

void GestureAuthDialog::setPassword(const QString password)
{
    //nothing
}

QString GestureAuthDialog::getPassword()
{
    //nothing
}

bool GestureAuthDialog::checkPassword()
{
    //nothing
}
