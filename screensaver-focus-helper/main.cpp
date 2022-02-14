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

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QString>
#include <QCursor>
#include <QPalette>
#include <QDesktopWidget>
#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QWidget w;
    w.resize(QApplication::desktop()->size());
    w.setWindowFlags(Qt::X11BypassWindowManagerHint);

    QPalette pal(w.palette());
    pal.setColor(QPalette::Background, Qt::black); //设置背景黑色
    w.setAutoFillBackground(true);
    w.setPalette(pal);
    w.setCursor(Qt::BlankCursor);
    w.showFullScreen();
    a.exec();

    return 0;
}
