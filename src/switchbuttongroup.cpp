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
#include "switchbuttongroup.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QPainter>
#include <QLabel>
#include <QSplitter>
#include <QLine>
#include <QIcon>

SwitchButtonGroup::SwitchButtonGroup(QWidget *parent) : QWidget(parent),
    m_checkedId(0)
{
    this->resize(264, 48);
    m_bg_switch = new QButtonGroup(this);
    m_pb_password = new QPushButton(tr("uEduPWD"), this);
    m_pb_qrCode = new QPushButton(tr("Wechat"), this);
    m_pb_password->setIcon(QIcon(":/image/assets/intel/pin.png"));
    m_pb_qrCode->setIcon(QIcon(":/image/assets/intel/wechat.png"));

    m_pb_password->setCheckable(true);
    m_pb_qrCode->setCheckable(true);

    m_pb_password->setStyleSheet("font-size:16px;color:#ffffff");
    m_pb_qrCode->setStyleSheet("font-size:16px;color:#ffffff");

    m_bg_switch->addButton(m_pb_password, 0);
    m_bg_switch->addButton(m_pb_qrCode, 1);

    QObject::connect(m_pb_password, &QPushButton::clicked, this, &SwitchButtonGroup::parseButtonStatus);
    QObject::connect(m_pb_qrCode,  &QPushButton::clicked, this, &SwitchButtonGroup::parseButtonStatus);

    initUI();
}

void SwitchButtonGroup::initUI()
{
    QHBoxLayout *hbLayout = new QHBoxLayout(this);
    hbLayout->setContentsMargins(0, 0, 0, 0);
    hbLayout->addSpacerItem(new QSpacerItem(40,20,QSizePolicy::Expanding, QSizePolicy::Maximum));
    hbLayout->addWidget(m_pb_password);
    hbLayout->addSpacerItem(new QSpacerItem(64,20,QSizePolicy::Expanding, QSizePolicy::Maximum));

    QLine *s = new QLine(this->width()/2, 0, this->width(), this->height()/2);
//    hbLayout->addChildWidget(s);

    hbLayout->addWidget(m_pb_qrCode);
    hbLayout->addSpacerItem(new QSpacerItem(40,20,QSizePolicy::Expanding, QSizePolicy::Maximum));
}

void SwitchButtonGroup::parseButtonStatus()
{
//    qDebug() << "---------------------------parseButtonStatus check id=" << m_bg_switch->checkedId();
    if (m_checkedId == m_bg_switch->checkedId())
        return;
    m_checkedId = m_bg_switch->checkedId();
//    switch (m_checkedId) {
//    case 0:
//        m_pb_password->setStyleSheet("font-size:16px;color:#2fb4e8");
//        m_pb_qrCode->setStyleSheet("font-size:16px;color:#ffffff");
//        break;
//    case 1:
//        m_pb_qrCode->setStyleSheet("font-size:16px;color:#2fb4e8");
//        m_pb_password->setStyleSheet("font-size:16px;color:#ffffff");
//        break;
//    default:
//        break;
//    }
    repaint();
    emit onSwitch(m_checkedId);
}

void SwitchButtonGroup::paintEvent(QPaintEvent *event)
{
//    Q_UNUSED(event);
//    QPainter painter(this);  //QWidget为绘图设备，创建一个画刷对象，主要用到设置颜色和填充模式，brush，setBrush
//    int width=this->width();  //获取QWidget 窗口的宽度
//    int height=this->height();//获取QWidget 窗口的高度

//    painter.setPen(QColor("#ffffff"));
//    painter.drawLine(this->width()/2, 0, this->width()/2, this->height()/2);

//    QPainterPath drawtriangle;  //单独画三角形

//    QPushButton *checkedButton;
//    if (m_checkedId == 0)
//    {
//        checkedButton = m_pb_password;
//    } else {
//        checkedButton = m_pb_qrCode;
//    }

//    drawtriangle.moveTo(checkedButton->x() + checkedButton->geometry().width()/2 -6, height);//左下角，第一点坐标为（0,height）;
//    drawtriangle.lineTo(checkedButton->x() + checkedButton->geometry().width()/2, height -12);//第二点坐标为（width/4,height/2）
//    drawtriangle.lineTo(checkedButton->x() + checkedButton->geometry().width()/2 + 6, height);//第三点坐标为（width/2,height）
//    painter.setPen(Qt::NoPen);
//    painter.setBrush(QColor("#2fb4e8"));   //填充绿色
//    painter.drawPath(drawtriangle);  //绘制出图形

    //设计稿变化重新做界面
    QPainter painter(this);
    QColor color("#ffffff");
    color.setAlphaF(0.15);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(color);
    painter.setPen(pen);
    painter.setBrush(color);

    if(m_checkedId == 0) {
        QRect rect1(4, 0, 120, 48);
        painter.drawRoundedRect(rect1, 24, 24);
    } else {
        QRect rect2(140, 0, 120, 48);
        painter.drawRoundedRect(rect2, 24, 24);
    }
}
