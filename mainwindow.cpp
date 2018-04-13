#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <QHBoxLayout>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::constructUI()
{
	ui->setupUi(this);
	createXScreensaverWidgets();
	screenState = LOCKSCREEN;
	xscreensaverPID = -1;
	setXScreensaverVisible(false);
	setRealTimeMouseTracking();
	/* Install event filter to capture keyboard and mouse event */
	installEventFilter(this);
	show();
}

void MainWindow::FSMTransition()
{
	constructUI();
}

void MainWindow::createXScreensaverWidgets()
{
	widgetXScreensaver = new QWidget();
	ui->hLayoutCentralWidget->addWidget(widgetXScreensaver);
	winId = widgetXScreensaver->winId();
}

void MainWindow::setRealTimeMouseTracking()
{
	/*
	 * setMouseTracking should be set for all child widgets, otherwise it
	 * won't work.
	 * http://www.eastfist.com/qt_tutorials/index.php/2013/08/28/
	 * solution-qt-setmousetracking-doesnt-work/
	 */
	setMouseTracking(true);
	ui->centralWidget->setMouseTracking(true);
	widgetXScreensaver->setMouseTracking(true);
}

/* All events are dispatched in this function */
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	(void)watched;
	switch (event->type()) {
	case QEvent::KeyPress:
		handleKeyPressEvent((QKeyEvent *)event);
		break;
	case QEvent::MouseMove:
		handleMouseMoveEvent((QMouseEvent *)event);
		break;
	default:
		break;
	}
	return false;
}

/* Key Press Event */
void MainWindow::handleKeyPressEvent(QKeyEvent *event)
{
	if (screenState == LOCKSCREEN) {
		if (event->key() == Qt::Key_Escape)
			switchToXScreensaver();
	} else { /* currentState == XSCREENSAVER */
		switchToLockScreen();
	}
}

/* Mouse Move Event */
void MainWindow::handleMouseMoveEvent(QMouseEvent *event)
{
	(void)event;
	if (screenState == LOCKSCREEN)
		return;
	switchToLockScreen();
}

/* Kill the xscreensaver process and show the lock screen */
void MainWindow::switchToLockScreen()
{
	int childStatus;
	if (xscreensaverPID == -1) /* Just in case */
		return;
	kill(xscreensaverPID, SIGKILL);
	waitpid(xscreensaverPID, &childStatus, 0);
	xscreensaverPID = -1;
	setXScreensaverVisible(false);
	screenState = LOCKSCREEN;
}

/* Start a xscreensaver process and embed it onto the widgetXScreensaver widget */
void MainWindow::switchToXScreensaver()
{
	embedXScreensaver();
	setXScreensaverVisible(true);
	screenState = XSCREENSAVER;
}

void MainWindow::setXScreensaverVisible(bool visible)
{
	widgetXScreensaver->setVisible(visible);
	ui->widgetLockScreen->setVisible(!visible);
	/*
	 * Move focus from lineedit to MainWindow object when xscreensaver is
	 * started, otherwise the eventFilter won't be invoked.
	 */
	if (visible)
		this->setFocus();
	else
		ui->lineEditPassword->setFocus();
}

void MainWindow::embedXScreensaver()
{
	char winIdStr[16] = {0};
	sprintf(winIdStr, "%lu", winId);
	xscreensaverPID = fork();
	if (xscreensaverPID == 0) {
		execl("/usr/lib/xscreensaver/binaryring", "xscreensaver",
					"-window-id", winIdStr, (char *)0);
		qDebug() << "execle failed. Can't start xscreensaver.";
	} else {
		qDebug() << "child pid=" << xscreensaverPID;
	}
}
