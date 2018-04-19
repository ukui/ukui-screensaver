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
	void createXScreensaverWidgets();
	void setRealTimeMouseTracking();
	void embedXScreensaver();
	void handleKeyPressEvent(QKeyEvent *event);
	void handleMouseMoveEvent(QMouseEvent *event);
	void setXScreensaverVisible(bool visible);
	void switchToLockScreen();
	void switchToXScreensaver();
	bool eventFilter(QObject *watched, QEvent *event);
	void constructUI();
	void closeEvent(QCloseEvent *event);
	void uiGetReady(bool ready);

public slots:
	void FSMTransition(); /* Transition FSM states according to signal */

private slots:
	void onUnlockClicked();
	void onPasswordEnter();

private:
	Ui::MainWindow *ui;
	QWidget *widgetXScreensaver;
	unsigned long int winId;
	enum ScreenState screenState;
	int xscreensaverPID;
	enum ProgramState programState;
	int toAuthChild[2];
	int toParent[2];
	int authPID;
};

#endif // MAINWINDOW_H
