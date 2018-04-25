#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pam.h"
#include "auxiliary.h"
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QTimer>
#include <QDesktopWidget>
#include <QScreen>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <QDebug>
extern "C" {
	#include <security/_pam_types.h>
}

#define GSETTINGS_SCHEMA_SCREENSAVER "org.ukui.screensaver"
#define KEY_MODE "mode"
#define KEY_THEMES "themes"
#define XSCREENSAVER_DIRNAME "/usr/lib/xscreensaver"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	qgsettings = new QGSettings(GSETTINGS_SCHEMA_SCREENSAVER);
	connect(qgsettings, &QGSettings::valueChanged, this, &MainWindow::onConfigurationChanged);
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
	screenState = LOCKSCREEN;
	setRealTimeMouseTracking();
	/* Install event filter to capture keyboard and mouse event */
	installEventFilter(this);
	setWindowStyle();
	show();
	/*
	 * After setting X11BypassWindowManagerHint flag, setFocus can't make
	 * LineEdit get focus, so we need to activate window manually.
	 */
	activateWindow();
}

void MainWindow::setWindowStyle()
{
	/* Calculate the total size of multi-head screen */
	int totalWidth = 0, totalHeight = 0;
	Q_FOREACH (QScreen *screen, QGuiApplication::screens()) {
		totalWidth += screen->geometry().width();
		totalHeight += screen->geometry().height();
	}
	setGeometry(0, 0, totalWidth, totalWidth); /* Full screen */

	/* Move lockscreen according to cursor position */
	lockscreenFollowCursor(QCursor::pos());

	setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
						| Qt::X11BypassWindowManagerHint);
}

#define AUTH_STATUS_LENGTH 16
void MainWindow::FSMTransition()
{
	struct pam_message_object pam_msg_obj;
	char auth_status_buffer[AUTH_STATUS_LENGTH];
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
		PIPE_OPS_SAFE(
			::read(toParent[0], &pam_msg_obj, sizeof(pam_msg_obj));
		);
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
		password = get_char_pointer(ui->lineEditPassword->text());
		PIPE_OPS_SAFE(
			::write(toAuthChild[1], password, strlen(password) + 1);
		);
		free(password);
		programState = WAIT_AUTH_STATUS;
		qDebug() << "User has input the password. Next state: WAIT_AUTH_STATUS.";
		break;
	case WAIT_AUTH_STATUS: /* pam_authenticate has returned */
		PIPE_OPS_SAFE(
			::read(toParent[0], auth_status_buffer, AUTH_STATUS_LENGTH);
		);
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
	if (screenState == LOCKSCREEN) {
		lockscreenFollowCursor(event->pos());
	} else {
		switchToLockScreen();
	}
}

/* lockscreen follows cursor */
void MainWindow::lockscreenFollowCursor(QPoint cursorPoint)
{
	QScreen *screen = NULL;
	Q_FOREACH (screen, QGuiApplication::screens()) {
		if (screen->geometry().contains(cursorPoint))
			break;
	}
	int x = screen->geometry().x() + (screen->geometry().width() -
				ui->widgetLockScreen->geometry().width()) / 2;
	int y = 0 + (screen->geometry().height() -
				ui->widgetLockScreen->geometry().height()) / 2;
	ui->widgetLockScreen->move(x, y);
}

/* Kill the xscreensaver process and show the lock screen */
void MainWindow::switchToLockScreen()
{
	int childStatus;
	Q_FOREACH (int xscreensaverPID, xscreensaverPIDList) {
		kill(xscreensaverPID, SIGKILL);
		waitpid(xscreensaverPID, &childStatus, 0);
	}
	xscreensaverPIDList.clear();
	Q_FOREACH (QWidget *widgetXScreensaver, widgetXScreensaverList) {
		widgetXScreensaver->deleteLater();
	}
	widgetXScreensaverList.clear();

	ui->widgetLockScreen->show();
	ui->lineEditPassword->setFocus();
	screenState = LOCKSCREEN;
}

/* Start a xscreensaver process and embed it onto the widgetXScreensaver widget */
void MainWindow::switchToXScreensaver()
{
	embedXScreensaver();
	ui->widgetLockScreen->hide();
	/*
	 * Move focus from lineedit to MainWindow object when xscreensaver is
	 * started, otherwise the eventFilter won't be invoked.
	 */
	this->setFocus();
	screenState = XSCREENSAVER;
}

/* Embed xscreensavers to each screen */
void MainWindow::embedXScreensaver()
{
	char *xscreensaver_path = get_char_pointer(getXScreensaver());
	for (int i = 0; i < QGuiApplication::screens().count(); i++) {
		/* Create widget for embedding the xscreensaver */
		QWidget *widgetXScreensaver = new QWidget(ui->centralWidget);
		widgetXScreensaverList.append(widgetXScreensaver);
		widgetXScreensaver->show();
		widgetXScreensaver->setMouseTracking(true);
		/* Move to top left corner at screen */
		QScreen *screen = QGuiApplication::screens()[i];
		widgetXScreensaver->setGeometry(screen->geometry());
		/*
		 * If the screensaver is black screen, we don't need to fork a
		 * separate process. What we need is just setting the background
		 * color of widgetXScreensaver. When switching to lockscreen,
		 * no process will be killed because xscreensaverPIDList is empty.
		 */
		if (strcmp(xscreensaver_path, "blank-only") == 0) {
			widgetXScreensaver->setStyleSheet(
						"background-color: black;");
			continue;
		}
		/* Get winId from widget */
		unsigned long winId = widgetXScreensaver->winId();
		char winIdStr[16] = {0};
		sprintf(winIdStr, "%lu", winId);
		/* Fork and execl */
		int xscreensaverPID = fork();
		if (xscreensaverPID == 0) {
			execl(xscreensaver_path, "xscreensaver", "-window-id",
							winIdStr, (char *)0);
			qDebug() << "execle failed. Can't start xscreensaver.";
		} else {
			xscreensaverPIDList.append(xscreensaverPID);
			qDebug() << "xscreensaver child pid=" << xscreensaverPID;
		}
	}
	free(xscreensaver_path);
}

/* Get xscreensaver path */
QString MainWindow::getXScreensaver()
{
	QString mode = qgsettings->getString(KEY_MODE);
	QList<QString> themes = qgsettings->getStringList(KEY_THEMES);
	QString selectedTheme;
	if (mode == "single") {
		selectedTheme = themes[0];
	} else if (mode == "random"){
		int randomIndex = qrand() % (themes.count());
		selectedTheme = themes[randomIndex];
	} else if (mode == "blank-only") { /* Note: blank not black */
		return QString("blank-only");
	} else {
		qDebug() << "Fatal error: unrecognized screensaver mode";
	}
	/* screensavers-binaryring => binaryring */
	selectedTheme = selectedTheme.split("-")[1];
	return QString("%1/%2").arg(XSCREENSAVER_DIRNAME, selectedTheme);
}



/*
 * GSettings
 */



void MainWindow::onConfigurationChanged(QString key)
{
}
