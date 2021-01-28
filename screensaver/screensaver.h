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
#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QGSettings>
#include <QString>
#include <QPushButton>
#include <QSettings>
#include <QSplitterHandle>
#include <QSplitter>
#include "sleeptime.h"
#include "chinesedate.h"
#include "mbackground.h"
#include "checkbutton.h"
class Screensaver : public QWidget
{
    Q_OBJECT

public:
    explicit Screensaver(QWidget *parent = 0);
    ~Screensaver();

private:
    void initUI();
    void setDatelayout();
    void setSleeptime();
    void setCenterWidget();
    void updateDate();
    void setUpdateBackground();
    void setUpdateCenterWidget();
    QString getDefaultBackground(QString background);


    QLabel *dateOfWeek;
    QLabel *dateOfLocaltime;
    QLabel *dateOfDay;
    QLabel *dateOfLunar;

    QWidget *centerWidget;

    QLabel *ubuntuKylinlogo;

    QWidget *timeLayout;
    SleepTime *sleepTime;
    QTimer *timer;
    ChineseDate *date;

    QGSettings *settings;
    QPixmap background;
    QString defaultBackground;

    QPushButton *escButton;
    QPushButton *settingsButton;
    QPushButton *WallpaperButton;
    QWidget *buttonWidget;
    QSettings *qsettings;

    MBackground *m_background;
    QString m_backgroundPath;

    QLabel *centerlabel1;
    QLabel *centerlabel2;
    QLabel *authorlabel;

    checkButton *checkSwitch;
    QLabel *autoSwitchLabel;
    QFrame *autoSwitch;

    QFrame *vboxFrame;
    bool isAutoSwitch;
    QTimer *m_timer;
    QGSettings *defaultSettings;

    int flag;
    bool hasChanged;
    int timeType;
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    void updateTime();
    void setDesktopBackground();
    void updateBackground();
    void updateCenterWidget(int index);
};

#endif // MAINWINDOW_H
