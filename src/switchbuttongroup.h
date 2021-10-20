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
#ifndef SWITCHBUTTONGROUP_H
#define SWITCHBUTTONGROUP_H

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>

class SwitchButtonGroup : public QWidget
{
    Q_OBJECT
public:
    explicit SwitchButtonGroup(QWidget *parent = nullptr);

Q_SIGNALS:
    void onSwitch(int id);

private Q_SLOTS:
    void parseButtonStatus();

private:
    void initUI();

    int m_checkedId;//上一次选中的按钮id
    QButtonGroup *m_bg_switch;
    QPushButton *m_pb_password;
    QPushButton *m_pb_qrCode;

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // SWITCHBUTTONGROUP_H
