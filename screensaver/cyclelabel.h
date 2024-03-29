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

#ifndef CYCLELABEL_H
#define CYCLELABEL_H
#include <QWidget>
#include <QFrame>
#include <QPainter>
#include <QSize>
#include <QPainterPath>
#include <QEvent>

class CycleLabel : public QFrame
{
    Q_OBJECT
public:
    CycleLabel(QWidget *parent = 0);
    void setSize(QSize size);

protected:

    void paintEvent(QPaintEvent *);

private:
    bool checked;

    QColor borderColorOff;
    QColor bgColorOff;
    QColor bgColorOn;
    QColor sliderColorOff;
    QColor sliderColorOn;
    int space; //滑块离背景间隔
    int rectRadius; //圆角角度
    int step; //移动步长
    int startX;
    int endX;
    QTimer * timer;

    void drawBg(QPainter *painter);
    void drawSlider(QPainter *painter);
    int m_width;
    int m_height;


};

#endif // CHECKBUTTON_H
