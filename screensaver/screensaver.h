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
#include <QTime>

#include "sleeptime.h"
#include "chinesedate.h"
#include "mbackground.h"
#include "checkbutton.h"
#include "scconfiguration.h"
#include "cyclelabel.h"
#include "weathermanager.h"

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
    void setWeatherLayout();
    void setNoticeLaout();
    void setSleeptime(bool Isshow);
    void setCenterWidget();
    void updateDate();
    void setUpdateBackground();
    void setUpdateCenterWidget();
    void startSwitchImages();
    void stopSwitchImages();
    void onBackgroundChanged();
    void setRandomText();
    void setPreviewText(bool bVisible);//设置预览样式
    void setRandomPos();
    void connectSingles();
    void updateBackgroundPath();
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
   
    QTimer              *switchTimer;
    QTimer              *fadeTimer;
    QStringList         imagePaths;
    QString             backgroundPath;
    int                 cycleTime;
    float               opacity;
    bool                isCustom;
    bool                isShowRestTime;
    bool                textIsCenter;
    QString             myText;
    QLabel              *myTextLabel;
    QLabel              *myPreviewLabel;
    CycleLabel          *cycleLabel;

    SCConfiguration     *configuration;
    void showNotice();
    void hideNotice();

    QWidget             *myTextWidget;
    QLabel              *dateOfLocaltime;

    QLabel *dateOfWeek;
    QLabel *dateOfLocaltimeHour;
    QLabel *dateofLocaltimeColon;
    QLabel *dateOfLocaltimeMinute;
    QLabel *dateOfDay;
    QLabel *dateOfLunar;
    QWidget             *centerWidget;

    QWidget             *timeLayout;
    SleepTime           *sleepTime;
    QTimer              *timer;

    QPixmap             background;

    QPushButton         *settingsButton;
    QPushButton         *WallpaperButton;
    QWidget             *buttonWidget;
    QSettings           *qsettings;

    MBackground         *m_background;

    QLabel              *centerlabel1;
    QLabel              *centerlabel2;
    QLabel              *authorlabel;

    checkButton         *checkSwitch;
    QLabel              *autoSwitchLabel;
    QFrame              *autoSwitch;

    QFrame              *vboxFrame;
    bool                isAutoSwitch;

    int                 flag;
    bool                hasChanged;
    int                 timeType;
    QString             dateType;
    QProcess            *process;
    QLabel              *screenLabel;
    bool                respondClick;
    static QTime        m_currentTime;

    WeatherManager *m_weatherManager;
    QWidget *m_weatherLaout;
    QLabel *m_weatherIcon;
    QLabel *m_weatherArea;
    QLabel *m_weatherCond;
    QLabel *m_weatherTemperature;

    QWidget *m_widgetNotice;
    QLabel *m_labelNoticeIcon;
    QLabel *m_labelNoticeMessage;
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
    void getWeatherFinish(QString city, QString cond, QString tmp);
    void onMessageNumberChanged(int num);
    void onMessageShowEnabledChanged(bool enabled);
    void onTimeFormatChanged(int type);
    void onDateFormatChanged(QString type);
    QPixmap getPaddingPixmap();
};

#endif // MAINWINDOW_H
