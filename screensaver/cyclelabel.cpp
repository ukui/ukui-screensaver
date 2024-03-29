/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "cyclelabel.h"

CycleLabel::CycleLabel(QWidget *parent)
    :QFrame(parent),
      m_width(20),
      m_height(20)
{
    this->setFixedSize(QSize(m_width, m_height));
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void CycleLabel::paintEvent(QPaintEvent *){
    //启用反锯齿
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    drawBg(&painter);
    drawSlider(&painter);
}

void CycleLabel::setSize(QSize size)
{
    m_width = size.width();
    m_height = size.height();
    setFixedSize(size);
    repaint();
}

void CycleLabel::drawBg(QPainter *painter)
{
    painter->save();

    if (!checked){
        painter->setPen(QColor(0,0,0,66));
        painter->setBrush(QColor(0,0,0,66));
    }
    else{
        painter->setPen(Qt::NoPen);
        painter->setBrush(bgColorOn);
    }

    //circle in
    QRect rect(0, 0, width(), height());
    //半径为高度的一半
    int radius = rect.height() / 2;
    //圆的宽度为高度
    int circleWidth = rect.height();

    QPainterPath path;
    path.moveTo(radius, rect.left());
    path.arcTo(QRectF(rect.left(), rect.top(), circleWidth, circleWidth), 90, 180);
    path.lineTo(rect.width() - radius, rect.height());
    path.arcTo(QRectF(rect.width() - rect.height(), rect.top(), circleWidth, circleWidth), 270, 180);
    path.lineTo(radius, rect.top());

    painter->drawPath(path);

    painter->restore();

}

void CycleLabel::drawSlider(QPainter *painter){
    painter->save();
    painter->setPen(Qt::NoPen);

    painter->setBrush(QColor("#ffffff"));

    //circle in
    QRect rect(0, 0, m_width, m_height);
    int sliderWidth = m_width/2;
    QRect sliderRect(sliderWidth/2, sliderWidth/2, sliderWidth, sliderWidth);
    painter->drawEllipse(sliderRect);

    painter->restore();
}


