#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

public slots:
	void FSMTransition(); /* Transition FSM states according to signal */

private slots:
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
};

#endif // MAINWINDOW_H
