#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <gsettings.h>
#include <QMainWindow>
#include "configuration.h"

namespace Ui {
class MainWindow;
}

enum ScreenState {
	LOCKSCREEN,
	XSCREENSAVER
};

enum ProgramState {
	IDLE,
	SHOW_PAM_MESSAGE,
	GET_PASSWORD,
	WAIT_AUTH_STATUS,
	AUTH_FAILED
};

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	void setRealTimeMouseTracking();
	void embedXScreensaver();
	void handleKeyPressEvent(QKeyEvent *event);
	void handleMouseMoveEvent(QMouseEvent *event);
	void switchToLockScreen();
	void switchToXScreensaver();
	bool eventFilter(QObject *watched, QEvent *event);
	void constructUI();
	void closeEvent(QCloseEvent *event);
	void uiGetReady(bool ready);
	void setWindowStyle();
	void lockscreenFollowCursor(QPoint cursorPosition);

public Q_SLOTS:
	void FSMTransition(); /* Transition FSM states according to signal */

private Q_SLOTS:
	void onUnlockClicked();
	void onPasswordEnter();

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
};

#endif // MAINWINDOW_H
