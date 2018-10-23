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
#ifndef SCREENSAVERWIDGET_H
#define SCREENSAVERWIDGET_H

#include <QWidget>
#include "screensaver.h"


class ScreenSaverWidget : public QWidget
{
    Q_OBJECT
public:
    ScreenSaverWidget(ScreenSaver *screensaver,
                      QWidget *parent = nullptr);
    ~ScreenSaverWidget() {qDebug() << "~ScreenSaverWidget";}

protected:
    void closeEvent(QCloseEvent *);
    void paintEvent(QPaintEvent *event);

private:
    void embedXScreensaver(const QString &path);

private Q_SLOTS:
    void onBackgroundChanged(const QString &path);

private:
    QTimer          *timer;
    int             xscreensaverPid;
    ScreenSaver     *screensaver;
    bool            closing;
    float           opacity;
};

#endif // SCREENSAVERWIDGET_H
