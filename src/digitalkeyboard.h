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

#ifndef DIGITALKEYBOARD_H
#define DIGITALKEYBOARD_H

#include <QWidget>
#include <QPushButton>
#include <QKeyEvent>

class DigitalKeyBoard : public QWidget
{
    Q_OBJECT

public:
    explicit DigitalKeyBoard(QWidget *parent = nullptr);

public Q_SLOTS:
    void onKeyReleaseEvent(QKeyEvent *event);

Q_SIGNALS:
    void numbersButtonPress(int btn_id);

private:
    void initUI();
    void initConnect();
    void setQSS();

private:
    QPushButton *m_pNumerPressBT[12]; // 0~9 是数字按键 10是删除键 11是清空键
};

#endif // DIGITALKEYBOARD_H

