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
**/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QWidget>
#include <QtDBus/QDBusInterface>
#include "configuration.h"
//#include "bioAuthentication/biodeviceview.h"
#include <QWindow>
#include <QScreen>
#include <QDateTime>
#include "monitorwatcher.h"
#include "types.h"

namespace Ui {
class MainWindow;
}

class VirtualKeyboard;
class AuthDialog;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void closed();

private Q_SLOTS:
//    void onScreenResized(int);
//    void onScreenCountChanged();

private:
    void initUI();
    void lockscreenFollowCursor(QPoint cursorPosition);

private:
    Ui::MainWindow      *ui;
    AuthDialog          *authDialog;
    VirtualKeyboard     *vKeyboard;
    Configuration       *configuration;
    MonitorWatcher      *monitorWatcher;
    QTimer              *timer;
    QPixmap             pixmap;
};

#endif // MAINWINDOW_H
