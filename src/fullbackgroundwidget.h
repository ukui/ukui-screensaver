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
#ifndef FULLBACKGROUNDWIDGET_H
#define FULLBACKGROUNDWIDGET_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QWidget>
#include "types.h"
#include <QAbstractNativeEventFilter>
#include <QDBusUnixFileDescriptor>
#include "logind.h"

void x11_get_screen_size(int *width,int *height);

class LockWidget;
class XEventMonitor;
class MonitorWatcher;
class Configuration;
class QDBusInterface;

class FullBackgroundWidget : public QWidget , public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit FullBackgroundWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void onScreensaver();
    void onBlankScreensaver();

public Q_SLOTS:
    void onCursorMoved(const QPoint &pos);
    void lock();
    void showLockWidget();
    void showScreensaver();
    int onSessionStatusChanged(uint status);
    void inhibit();
    void uninhibit();

private:
    void init();
    void clearScreensavers();
    bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    void onScreenCountChanged(int);
    void onDesktopResized();
    void onGlobalKeyPress(const QString &key);
    void onGlobalKeyRelease(const QString &key);
    void onGlobalButtonDrag(int xPos, int yPos);
    void onPrepareForSleep(bool sleep);
    void switchToLinux();
    void laterActivate();
    void setLockState();
    void laterInhibit(bool val);
    void laterStartAuth();

private:
    QDBusInterface      *smInterface;
    LockWidget          *lockWidget;
    XEventMonitor       *xEventMonitor;
    MonitorWatcher      *monitorWatcher;
    Configuration       *configuration;
    QList<QWidget*>     widgetXScreensaverList;
    QList<pid_t>        xscreensaverPidList;
    bool                isLocked;
    ScreenStatus        screenStatus;
    QPixmap             background;
    bool                lockState;
    QDBusUnixFileDescriptor m_inhibitFileDescriptor;
};

#endif // FULLBACKGROUNDWIDGET_H
