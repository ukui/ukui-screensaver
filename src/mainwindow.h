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

#include <QMainWindow>
#include <QtDBus/QDBusInterface>
#include "configuration.h"
#include "bioAuthentication/biodeviceview.h"
#include <QWindow>
#include <QScreen>
#include "monitorwatcher.h"

namespace Ui {
class MainWindow;
}

enum ScreenState {
	UNDEFINED,
	LOCKSCREEN,
	XSCREENSAVER,
	XSCREENSAVER_BY_IDLE /* Xscreensaver is activated by session idle */
};

enum ProgramState {
	IDLE,
	SHOW_PAM_MESSAGE,
	GET_PASSWORD,
	WAIT_AUTH_STATUS,
	AUTH_FAILED
};

/* https://www.narf.ssji.net/~shtrom/wiki/projets/gnomescreensavernosession */
enum SessionStatus {
	SESSION_AVAILABLE = 0,
	SESSION_INVISIBLE = 1,
	SESSION_BUSY = 2,
	SESSION_IDLE = 3
};

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
    void setShowSaver(bool showSaver);
    void showDialog();

private:
	void setRealTimeMouseTracking();
	void embedXScreensaver();
	void handleKeyPressEvent(QKeyEvent *event);
	void handleMouseMoveEvent(QMouseEvent *event);
	void switchToLockscreen();
	void switchToXScreensaver();
	void startXScreensaverWithoutAuth();
	void constructUI();
	void uiGetReady(bool ready);
	void setWindowStyle();
	void lockscreenFollowCursor(QPoint cursorPosition);
	QString getUserAvatarPath(QString username);
	bool signalSenderFilter(int signalSenderPID);
    void setPasswordVisible(bool visible);
    void setCapsLockWarn();
    void clearSavers();

protected:
	void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
//    void keyReleaseEvent(QKeyEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    void closed();

public Q_SLOTS:
	void FSMTransition(int signalSenderPID); /* Transition FSM states according to signal */
    void onScreenResized(int);
    void onScreenCountChanged();

private Q_SLOTS:
	void onUnlockClicked();
	void onPasswordEnter();
    void onSessionIdle();

    void on_btnSwitchUser_clicked();

public Q_SLOTS:
    void onGlobalKeyPress(int keyId);
    void onGlobalMouseMove(int x, int y);

private:
	Ui::MainWindow *ui;
	unsigned long int winId;
	enum ScreenState screenState;
	QList<int> xscreensaverPIDList;
	QList<QWidget *> widgetXScreensaverList;
	enum ProgramState programState;
	int toAuthChild[2];
	int toParent[2];
	int authPID;
	Configuration *configuration;
	QPixmap pixmap;
	QDBusInterface *interface;
    BioDeviceView *widgetBioDevices;
    bool isActivated;
    bool isPasswdFailed;
    QTimer *timer;
    bool showSaver;
    MonitorWatcher *monitorWatcher;
};

#endif // MAINWINDOW_H
