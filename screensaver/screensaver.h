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

#ifndef SCREENSAVER_H
#define SCREENSAVER_H
#include <QMainWindow>
#include <QProcess>
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
#include "scconfiguration.h"
#include "cyclelabel.h"

class Screensaver : public QWidget
{
    Q_OBJECT

public:
    explicit Screensaver(QWidget *parent = 0);
    ~Screensaver();
    void addClickedEvent();

private:
    void initUI();
    void setDatelayout();
    void setSleeptime(bool Isshow);
    void setCenterWidget();
    void updateDate();
    void setUpdateBackground();
    void setUpdateCenterWidget();
    void startSwitchImages();
    void stopSwitchImages();
    void onBackgroundChanged();
    void setRandomText();
    void setRandomPos();
    void connectSingles();
    void updateBackgroundPath();
   
    int                 imageIndex;
    QTimer              *switchTimer;
    QTimer              *fadeTimer;
    QStringList         imagePaths;
    QString             backgroundPath;
    QString             lastPath;
    QString             currentPath;
    int                 cycleTime;
    float               opacity;
    QPixmap             lastPixmap;
    QPixmap             currentPixmap;
    bool                isCustom;
    bool                isShowRestTime;
    bool                textIsCenter;
    QString             myText;
    QLabel              *myTextLabel;
    CycleLabel          *cycleLabel;

    SCConfiguration     *configuration;

    QWidget             *myTextWidget;
    QLabel              *dateOfWeek;
    QLabel              *dateOfLocaltime;
    QLabel              *dateOfDay;
    QLabel              *dateOfLunar;

    QWidget             *centerWidget;

    QLabel              *ubuntuKylinlogo;

    QWidget             *timeLayout;
    SleepTime           *sleepTime;
    QTimer              *timer;
    ChineseDate         *date;

    QPixmap             background;
    QString             defaultBackground;

    QPushButton         *escButton;
    QPushButton         *settingsButton;
    QPushButton         *WallpaperButton;
    QWidget             *buttonWidget;
    QSettings           *qsettings;

    MBackground         *m_background;
    QString             m_backgroundPath;

    QLabel              *centerlabel1;
    QLabel              *centerlabel2;
    QLabel              *authorlabel;

    checkButton         *checkSwitch;
    QLabel              *autoSwitchLabel;
    QFrame              *autoSwitch;

    QFrame              *vboxFrame;
    bool                isAutoSwitch;
    QTimer              *m_timer;

    int                 flag;
    bool                hasChanged;
    int                 timeType;
    QString             dateType;
    QProcess		*process;
    QLabel              *screenLabel;
    bool                respondClick;
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    void updateTime();
    void updateBackground();
    void updateCenterWidget(int index);
    void autoSwitchChanged(bool iswitch);
    void backgroundPathChanged(QString path);
    void cycleTimeChanged(int cTime);
    void myTextChanged(QString text);
    void showRestTimeChanged(bool isShow);
    void textIsCenterChanged(bool isCenter);
    void themeChanged();
};

#endif // MAINWINDOW_H
