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

#include "checkbutton.h"

checkButton::checkButton(QWidget *parent)
    :QFrame(parent)
{
    this->setFixedSize(QSize(52, 24));
    checked = false;

    borderColorOff = QColor("#cccccc");

    bgColorOff = QColor("#ffffff");
    bgColorOn = QColor("#0078d7");

    sliderColorOff = QColor("#cccccc");
    sliderColorOn = QColor("#ffffff");

    space = 4;


    step = width() / 50;
    startX = 0;
    endX= 0;

    timer = new QTimer(this);
    timer->setInterval(5);
    connect(timer, SIGNAL(timeout()), this, SLOT(updatevalue()));
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);


}

void checkButton::paintEvent(QPaintEvent *){
    //启用反锯齿
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    drawBg(&painter);
    drawSlider(&painter);
}


void checkButton::drawBg(QPainter *painter){
    painter->save();

    if (!checked){
        painter->setPen(QColor(255,255,255,30));
        painter->setBrush(QColor(255,255,255,30));
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


void checkButton::drawSlider(QPainter *painter){
    painter->save();
    painter->setPen(Qt::NoPen);

    if (!checked){
        painter->setBrush(sliderColorOff);
    }
    else
        painter->setBrush(sliderColorOn);

    //circle in
    QRect rect(0, 0, width(), height());
    int sliderWidth = rect.height() - space * 2;
    QRect sliderRect(startX + space, space, sliderWidth, sliderWidth);
    painter->drawEllipse(sliderRect);

    painter->restore();
}

void checkButton::mousePressEvent(QMouseEvent *){
    checked = !checked;
    emit checkedChanged(checked);

    step = width() / 50;

    if (checked){

        endX = width() - height();
    }
    else{
        endX = 0;
    }
    timer->start();
}

void checkButton::resizeEvent(QResizeEvent *){
    step = width() / 50;

    if (checked){
        startX = width() - height();
    }
    else
        startX = 0;

    update();
}

void checkButton::updatevalue(){
    if (checked)
        if (startX < endX){
            startX = startX + step;
        }
        else{
            startX = endX;
            timer->stop();
        }
    else{
        if (startX > endX){
            startX = startX - step;
        }
        else{
            startX = endX;
            timer->stop();
        }
    }
    update();
}


void checkButton::setChecked(bool checked){
    if (this->checked != checked){
        this->checked = checked;
        emit checkedChanged(checked);
        update();
    }

    step = width() / 50;

    if (checked){
        endX = width() - height();
    }
    else{
        endX = 0;
    }
    timer->start();
}

bool checkButton::isChecked(){
    return this->checked;
}

