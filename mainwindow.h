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

public slots:
	void FSMTransition(); /* Transition FSM states according to signal */

private:
	Ui::MainWindow *ui;
	QWidget *widgetXScreensaver;
	unsigned long int winId;
	enum ScreenState screenState;
	int xscreensaverPID;
};

#endif // MAINWINDOW_H
