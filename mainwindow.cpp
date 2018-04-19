#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pam.h"
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QTimer>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <QDebug>
extern "C" {
	#include <security/_pam_types.h>
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	hide();
	delete ui;
	ui = NULL;
	widgetXScreensaver->deleteLater();
	widgetXScreensaver = NULL;
	removeEventFilter(this);

	event->ignore(); /* No further processing */
	return;
}

void MainWindow::constructUI()
{
	ui = new Ui::MainWindow;
	ui->setupUi(this);
	connect(ui->lineEditPassword, &QLineEdit::returnPressed, this, &MainWindow::onPasswordEnter);
	connect(ui->btnUnlock, &QPushButton::clicked, this, &MainWindow::onUnlockClicked);
	createXScreensaverWidgets();
	screenState = LOCKSCREEN;
	xscreensaverPID = -1;
	setXScreensaverVisible(false);
	setRealTimeMouseTracking();
	/* Install event filter to capture keyboard and mouse event */
	installEventFilter(this);
	show();
}

#define AUTH_STATUS_LENGTH 3
void MainWindow::FSMTransition()
{
	struct pam_message_object pam_msg_obj;
	char auth_status_buffer[AUTH_STATUS_LENGTH];
	int read_count;
	int auth_status;
	char *password;
	switch (programState) { /* Current program state */
	case IDLE: /* Idle in background */
	case AUTH_FAILED: /* Re-Authenticate */
		if((pipe(toAuthChild) == -1) || (pipe(toParent) == -1)) {
			qDebug() << "Can't create pipe for authentication IPC.";
			break;
		}
		authPID = fork();
		if (authPID != 0) { /* Parent process */
			::close(toAuthChild[0]); /* Close read end */
			::close(toParent[1]); /* Close write end */
			/*
			 * During re-authenticating, the GUI has been there,
			 * we don't need to construct it again.
			 */
			if (programState == IDLE)
				constructUI();
			uiGetReady(true);
			programState = SHOW_PAM_MESSAGE;
			qDebug() << "UI is ready. Next state: SHOW_PAM_MESSAGE.";
		} else { /* Child process */
			qDebug() << "Authentication subprocess detached.";
			::close(toAuthChild[1]); /* Close write end */
			::close(toParent[0]); /* Close read end */
			/* Child process will invoke pam and will not return */
			authenticate(toParent, toAuthChild);
		}
		break;
	case SHOW_PAM_MESSAGE: /* Triggered by conversation function in pam.c */
		read(toParent[0], &pam_msg_obj, sizeof(pam_msg_obj));
		ui->lblPrompt->setText(QString::fromUtf8(pam_msg_obj.msg));
		if (pam_msg_obj.msg_style == PAM_PROMPT_ECHO_OFF) {
			ui->lineEditPassword->setEchoMode(QLineEdit::Password);
			programState = GET_PASSWORD;
			qDebug() << "PAM messages has been shown. Next state: GET_PASSWORD.";
		} else if (pam_msg_obj.msg_style == PAM_PROMPT_ECHO_ON){
			ui->lineEditPassword->setEchoMode(QLineEdit::Normal);
			programState = GET_PASSWORD;
			qDebug() << "PAM messages has been shown. Next state: GET_PASSWORD.";
		} else {
			qDebug() << "PAM only want to show message. Next state is still SHOW_PAM_MESSAGE.";
		}
		break;
	case GET_PASSWORD: /* Triggered by ENTER */
		password = ui->lineEditPassword->text().toLocal8Bit().data();
		write(toAuthChild[1], password, strlen(password));
		programState = WAIT_AUTH_STATUS;
		qDebug() << "User has input the password. Next state: WAIT_AUTH_STATUS.";
		break;
	case WAIT_AUTH_STATUS: /* pam_authenticate has returned */
		read_count = read(toParent[0], auth_status_buffer, AUTH_STATUS_LENGTH);
		auth_status_buffer[read_count] = 0;
		sscanf(auth_status_buffer, "%d", &auth_status);
		qDebug() << "auth_status:" << auth_status;
		if (auth_status == PAM_SUCCESS) {
			close();
			programState = IDLE;
			qDebug() << "Authenticate successfully. Next state: IDLE";
		} else {
			QTimer::singleShot(0, [this]{
				::raise(SIGUSR1);
			});
			programState = AUTH_FAILED;
			qDebug() << "Authenticate unsuccessfully. Next state: AUTH_FAILED.";
		}
		::close(toParent[0]);
		::close(toAuthChild[1]);
		waitpid(authPID, NULL, 0);
		qDebug() << "All done.";
		break;
	default:
		break;
	}
}

void MainWindow::onUnlockClicked()
{
	qDebug() << "Click unlock button.";
	uiGetReady(false);
	::raise(SIGUSR1);
}

void MainWindow::onPasswordEnter()
{
	qDebug() << "Press enter key.";
	uiGetReady(false);
	::raise(SIGUSR1);
}

void MainWindow::uiGetReady(bool ready)
{
	ui->lineEditPassword->setEnabled(ready);
	ui->btnUnlock->setEnabled(ready);
	if (ready)
		setCursor(Qt::ArrowCursor);
	else
		setCursor(Qt::BusyCursor);

	if (ready)
		ui->lineEditPassword->clear();

	if (ready)
		ui->lineEditPassword->setFocus();
}



/*
 * XScreensaver
 */

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
		qDebug() << "xscreensaver child pid=" << xscreensaverPID;
	}
}
