#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pam.h"
#include "auxiliary.h"
#include <QHBoxLayout>
#include <QTimer>
#include <QDesktopWidget>
#include <QScreen>
#include <QKeyEvent>
#include <QPainter>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <QDebug>
#include <QtDBus/QDBusReply>
#include "screensaverwidget.h"

extern "C" {
	#include <security/_pam_types.h>
}

#define DBUS_SESSION_MANAGER_SERVICE "org.gnome.SessionManager"
#define DBUS_SESSION_MANAGER_PATH "/org/gnome/SessionManager/Presence"
#define DBUS_SESSION_MANAGER_INTERFACE "org.gnome.SessionManager.Presence"

QString screenStates[] = {"UNDEFINED", "LOCKSCREEN", "XSCREENSAVER", "XSCREENSAVER_BY_IDLE"};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      widgetBioDevices(nullptr),
      isActivated(false),
      isPasswdFailed(false)
{
	configuration = new Configuration();
	programState = IDLE;
	screenState = UNDEFINED;
	/* Listen to SessionManager StatusChanged signal for idle activation */
	interface = new QDBusInterface(DBUS_SESSION_MANAGER_SERVICE,
					DBUS_SESSION_MANAGER_PATH,
					DBUS_SESSION_MANAGER_INTERFACE,
					QDBusConnection::sessionBus());
	connect(interface, SIGNAL(StatusChanged(unsigned int)),
				this, SLOT(sessionStatusChanged(unsigned int)));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	hide();
	delete ui;
    ui = nullptr;
    widgetBioDevices = nullptr;
	removeEventFilter(this);

    BioDevices devices;
    devices.clear();

	event->ignore(); /* No further processing */

    /* ungrab the control of mouse and keyboard events */
    closeGrab();

    isActivated = false;

    timer->stop();

	return;
}

void MainWindow::constructUI()
{
    qDebug() << "MainWindow::constructUI";
	ui = new Ui::MainWindow;
	ui->setupUi(this);
    ui->widgetLockscreen->setFixedSize(610, 200);
    /* Put the button in the LineEdit */
    QHBoxLayout *hLayoutPwd = new QHBoxLayout;
    hLayoutPwd->setSpacing(0);
    hLayoutPwd->setContentsMargins(1, 1, 1, 1);
    hLayoutPwd->addStretch();
    hLayoutPwd->addWidget(ui->lblCapsLock);
    hLayoutPwd->addWidget(ui->btnHidePwd);
    hLayoutPwd->addWidget(ui->btnUnlock);
    ui->btnUnlock->setFixedSize(70, 38);
    ui->btnUnlock->setFlat(true);
    ui->btnUnlock->setCursor(Qt::PointingHandCursor);
    ui->lineEditPassword->setLayout(hLayoutPwd);
    ui->lineEditPassword->setTextMargins(1, 1, ui->btnUnlock->width() +
                                         ui->btnHidePwd->width(), 1);
//    ui->lineEditPassword->setCursor(Qt::IBeamCursor);
    ui->lineEditPassword->setFocusPolicy(Qt::NoFocus);
    ui->lineEditPassword->hide();
    ui->lineEditPassword->installEventFilter(this);
    ui->btnBiometric->setIcon(QIcon(":/resource/fingerprint-icon.png"));
    ui->btnBiometric->setIconSize(QSize(40, 40));
    ui->btnBiometric->hide();
    ui->btnHidePwd->setFocusPolicy(Qt::NoFocus);
    ui->btnHidePwd->setCursor(Qt::ArrowCursor);
    ui->lblCapsLock->setPixmap(QPixmap(":/image/warn.png"));
    connect(ui->btnBiometric, &QPushButton::clicked, this,[&]{
        if(widgetBioDevices && widgetBioDevices->isHidden()) {
            widgetBioDevices->show();
            ui->lineEditPassword->hide();
            ui->btnBiometric->hide();
            programState = AUTH_FAILED;

            ::close(toParent[0]);
            ::close(toAuthChild[1]);
//            ::waitpid(authPID, NULL, 0);
            ::raise(SIGUSR1);
        }
    } );

    qDebug() << "Background: " << configuration->getBackground();
    pixmap.load(configuration->getBackground());
	/* Set avatar, password entry, button ... */
    QString username = ::getenv("USER");
	QPixmap avatarPixmap;
    avatarPixmap.load(getUserAvatarPath(username));
	avatarPixmap = avatarPixmap.scaled(128, 128, Qt::IgnoreAspectRatio);
	ui->lblAvatar->setPixmap(avatarPixmap);
    ui->lblUsername->setText(username);
    ui->lineEditPassword->setFixedSize(350, 40);
	ui->btnUnlock->setFixedHeight(40);
    ui->lblAvatar->setStyleSheet("border:2px solid white");
	ui->lblUsername->setStyleSheet("color: white; font-size: 23px;");
	ui->lblLogged->setStyleSheet("color: white; font-size: 13px;");
	ui->lblPrompt->setStyleSheet("color: white; font-size: 13px;");
    ui->lineEditPassword->setStyleSheet("border:1px solid #026096");
	ui->btnUnlock->setStyleSheet(
				"QPushButton {"
                    "border:0px;"
					"color: black;"
					"background-color: #0078d7;"
				"}"
				"QPushButton:hover {"
					"background-color: #3f8de0;"
				"}"
				"QPushButton:active {"
					"background-color: #2367b9;"
				"}"
				"QPushButton:disabled {"
					"background-color: #013C76;"
				"}");
    ui->btnBiometric->setStyleSheet(
                "QPushButton{"
                    "border: none;"
                    "outline: none;"
                "}"
                "QPushButton::hover{"
                    "background-color:rgb(0, 0, 0, 50);"
                "}");
    ui->btnHidePwd->setStyleSheet(
                "QPushButton{"
                    "border: none;"
                    "outline: none;"
                "}");

	connect(ui->lineEditPassword, &QLineEdit::returnPressed, this, &MainWindow::onPasswordEnter);
	connect(ui->btnUnlock, &QPushButton::clicked, this, &MainWindow::onUnlockClicked);
    connect(ui->btnHidePwd, &QPushButton::clicked, this, [&]{
        setPasswordVisible(ui->lineEditPassword->echoMode() == QLineEdit::Password);
    });
	screenState = LOCKSCREEN;
	setRealTimeMouseTracking();
	setWindowStyle();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&]{
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        ui->lblTime->setText(time);
    });

    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->lblTime->setText(time);
    ui->lblTime->setStyleSheet("QLabel{color:white; font-size: 55px;}");
    ui->lblTime->adjustSize();
    timer->start(1000);

    QString date = QDate::currentDate().toString("yyyy/MM/dd dddd");
    qDebug() << "current date: " << date;
    ui->lblDate->setText(date);
    ui->lblDate->setStyleSheet("QLabel{color:white; font-size: 20px;}");
    ui->lblDate->adjustSize();

	show();
    /* grab control of the mouse and keyboard events in lockscreen window  */
    if(!establishGrab())
        qWarning("can't grab mouse or keyboard");
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
    setAttribute(Qt::WA_DeleteOnClose);
}

/* Draw background image */
void MainWindow::paintEvent(QPaintEvent *event)
{
	(void)event;
	/*
	 * MainWindow is stretched to all screens. We can draw background image
	 * on it based on geometry of each screen.
	 */
	Q_FOREACH (QScreen *screen, QGuiApplication::screens()) {
		QPainter painter(this);
        painter.drawPixmap(screen->geometry(), pixmap);
	}
    return QWidget::paintEvent(event);
}


/* Verify if the sender of SIGUSR1 is correct or not */
bool MainWindow::signalSenderFilter(int senderSenderPid)
{
	if (senderSenderPid == getpid() || senderSenderPid == authPID)
		return true;
	/*
	 * If the sender is neither the current process nor the authentication
	 * child process, then we should check the programState.
	 * If programState is IDLE and we can read a number from a fifo and the
	 * data is the pid of the current process, then the signal is sent by
	 * ukui-screensaver-command.
	 */
	FILE *fp;
	int filedata;
    char file[1024];
    snprintf(file, sizeof(file), "%s/.cache/ukui-screensaver/lock", getenv("HOME"));
	if (programState == IDLE) {
		fp = fopen(file, "r");
		if (!fp) {
			qDebug() << "Can't open file";
			perror("Details: ");
			return false;
		}
        int i = fscanf(fp, "%d", &filedata);
        (void)i;    //eliminate warning
		if (filedata == getpid()) {
			qDebug() << "ukui-screensaver-command request for locking";
			return true;
		} else {
			qDebug() << "File data do not match the pid of ukui-screensaver";
			return false;
		}
	} else {
		qDebug() << "Receive a SIGUSR1 but programState is not IDLE."
				"Ignore it!";
		return false;
	}
}

void MainWindow::setPasswordVisible(bool visible)
{
    if(visible) {
        ui->lineEditPassword->setEchoMode(QLineEdit::Normal);
        ui->btnHidePwd->setIcon(QIcon(":/image/show-password.png"));
    } else {
        ui->lineEditPassword->setEchoMode(QLineEdit::Password);
        ui->btnHidePwd->setIcon(QIcon(":/image/hide-password.png"));
    }
}

#define AUTH_STATUS_LENGTH 16
void MainWindow::FSMTransition(int signalSenderPID)
{
	struct pam_message_object pam_msg_obj;
	char auth_status_buffer[AUTH_STATUS_LENGTH];
	int auth_status;
	char *password;

	if(!signalSenderFilter(signalSenderPID))
		return;

    isActivated = true;

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
        qDebug() << "PAM Message---"<< pam_msg_obj.msg;

        /* Check whether the pam message is from biometric pam module */
        if(strcmp(pam_msg_obj.msg, BIOMETRIC_PAM) == 0) {
            BioDevices devices;
            if(isPasswdFailed || devices.featuresNum(getuid()) <= 0) {
                qDebug() << "no avaliable device, enable password authentication";
                PIPE_OPS_SAFE(
                    ::write(toAuthChild[1], BIOMETRIC_IGNORE, strlen(BIOMETRIC_IGNORE) + 1);
                );
                programState = SHOW_PAM_MESSAGE;
                isPasswdFailed = false;
            } else {
                qDebug() << "enable biometric authentication";
                if(!widgetBioDevices) {
                    widgetBioDevices = new BioDeviceView(getuid(), ui->widgetLockscreen);
                    QRect widgetBioDevicesRect(ui->lineEditPassword->geometry().left(),
                                               ui->lineEditPassword->geometry().top() - 2,
                                               BIODEVICEVIEW_WIDTH, BIODEVICEVIEW_HEIGHT);
                    widgetBioDevices->setGeometry(widgetBioDevicesRect);
                    connect(widgetBioDevices, &BioDeviceView::backToPasswd, this, [&]{
                        widgetBioDevices->hide();
                        ui->btnBiometric->show();
                        ui->lblPrompt->setText("");
                        PIPE_OPS_SAFE(
                            ::write(toAuthChild[1], BIOMETRIC_IGNORE, strlen(BIOMETRIC_IGNORE) + 1);
                        );
                        programState = SHOW_PAM_MESSAGE;
                    });
                    connect(widgetBioDevices, &BioDeviceView::notify, this, [&](const QString &text){
                        ui->lblPrompt->setText(text);
                    });
                    connect(widgetBioDevices, &BioDeviceView::authenticationComplete, this, [&](bool ret){
                       if(ret) {
                           qDebug() << "authentication success";
                           PIPE_OPS_SAFE(
                               ::write(toAuthChild[1], BIOMETRIC_SUCESS, strlen(BIOMETRIC_SUCESS) + 1);
                           );
                           programState = WAIT_AUTH_STATUS;
                       }
                    });
                }
                widgetBioDevices->show();
            }
            break;
        }

        ui->lineEditPassword->show();
        ui->lineEditPassword->setFocus();

		if (pam_msg_obj.msg_style == PAM_PROMPT_ECHO_OFF) {
            setPasswordVisible(false);
			ui->lineEditPassword->setPlaceholderText(
					QString::fromUtf8(pam_msg_obj.msg));
			programState = GET_PASSWORD;
			qDebug() << "PAM messages has been shown. Next state: GET_PASSWORD.";
		} else if (pam_msg_obj.msg_style == PAM_PROMPT_ECHO_ON){
            setPasswordVisible(true);
			ui->lineEditPassword->setPlaceholderText(
					QString::fromUtf8(pam_msg_obj.msg));
			programState = GET_PASSWORD;
			qDebug() << "PAM messages has been shown. Next state: GET_PASSWORD.";
		} else {
			ui->lblPrompt->setText(QString::fromUtf8(pam_msg_obj.msg));
			qDebug() << "PAM only want to show message. Next state is still SHOW_PAM_MESSAGE.";
		}
		break;
	case GET_PASSWORD: /* Triggered by ENTER */
        qDebug() << "STATE---GET_PASSWORD";
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
            QTimer::singleShot(0, [&]{
				::raise(SIGUSR1);
			});
			ui->lblPrompt->setText(tr("Password Incorrect"));
            ui->lineEditPassword->hide();
            isPasswdFailed = true;
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


    if (ready) {
		ui->lineEditPassword->clear();
		ui->lineEditPassword->setFocus();
    }
}

void MainWindow::setCapsLockWarn()
{
    bool state = checkCapsLockState();
    ui->lblCapsLock->setVisible(state);
    int textMargin = ui->btnUnlock->width() + ui->btnHidePwd->width() +
            (state ? ui->lblCapsLock->width() : 0);
    ui->lineEditPassword->setTextMargins(1, 1, textMargin, 1);
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
    switch (event->type()) {
    case QEvent::KeyRelease:
        if(((QKeyEvent*)event)->key() == Qt::Key_CapsLock)
            setCapsLockWarn();
        break;
    case QEvent::FocusIn:
        if(watched == ui->lineEditPassword)
            setCapsLockWarn();
        break;
    default:
        break;
    }
    return false;
}

/* Key Press Event */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
	if (screenState == LOCKSCREEN) {
		if (event->key() == Qt::Key_Escape) {
            screenState = XSCREENSAVER;
			switchToXScreensaver();
		}
    }
    return QMainWindow::keyReleaseEvent(event);
}

/* Mouse Move Event */
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (screenState == LOCKSCREEN) {
        lockscreenFollowCursor(event->pos());
    }
    return QMainWindow::mouseMoveEvent(event);
}

void MainWindow::onGlobalKeyPress()
{
    if (screenState == XSCREENSAVER) {
        switchToLockscreen();
    } else if (screenState == XSCREENSAVER_BY_IDLE) {
        close();
        screenState = UNDEFINED;
    }
}

void MainWindow::onGlobalMouseMove()
{
    if (screenState == XSCREENSAVER) {
        switchToLockscreen();
    } else if (screenState == XSCREENSAVER_BY_IDLE) {
        close();
        screenState = UNDEFINED;
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
				ui->widgetLockscreen->geometry().width()) / 2;
	int y = 0 + (screen->geometry().height() -
				ui->widgetLockscreen->geometry().height()) / 2;
	ui->widgetLockscreen->move(x, y);

    x = screen->geometry().x();
    y = screen->geometry().y() + screen->geometry().height() - 150;
    ui->widgetTime->move(x, y);
}


/* Kill the xscreensaver process and show the lock screen */
void MainWindow::switchToLockscreen()
{
    qDebug() << "switch to lockscreen";
    for(auto widget : widgetXScreensaverList) {
        widget->close();
    }
    qDebug() << "show LockScreen";

	ui->lineEditPassword->setFocus();
    setCursor(Qt::ArrowCursor);
    ui->lineEditPassword->grabKeyboard();

    screenState = LOCKSCREEN;
}

/* Start a xscreensaver process and embed it onto the widgetXScreensaver widget */
void MainWindow::switchToXScreensaver()
{
    embedXScreensaver();
    setCursor(Qt::BlankCursor);
}

/* Embed xscreensavers to each screen */
void MainWindow::embedXScreensaver()
{
    qDebug() << "embedXScreensaver";
    widgetXScreensaverList.clear();
    ScreenSaver *saver = configuration->getScreensaver();
	for (int i = 0; i < QGuiApplication::screens().count(); i++) {
        ScreenSaverWidget *saverWidget = new ScreenSaverWidget(saver, this);
        widgetXScreensaverList.push_back(saverWidget);

        qDebug() << QGuiApplication::screens()[i]->geometry();
        saverWidget->setGeometry(QGuiApplication::screens()[i]->geometry());
	}
}

/* Listen to SessionManager StatusChanged D-Bus signal */
void MainWindow::sessionStatusChanged(unsigned int status)
{
	switch (status) {
	case SESSION_AVAILABLE:
		break;
	case SESSION_INVISIBLE:
		break;
	case SESSION_BUSY:
		break;
	case SESSION_IDLE:
        qDebug() << "session idle";
        /* skip if the lock window is show */
        if(isActivated) {
            if(screenState == LOCKSCREEN) {
                switchToXScreensaver();
                screenState = XSCREENSAVER;
            }
            break;
        }
		if (configuration->xscreensaverActivatedWhenIdle() &&
			configuration->lockWhenXScreensaverActivated()) {
            qDebug() << "run screensaver and lockscreen";
			/* Start authentication and construct UI */
			FSMTransition(getpid());
			switchToXScreensaver();
			screenState = XSCREENSAVER;
		} else if (configuration->xscreensaverActivatedWhenIdle()) {
            qDebug() << "only run screensaver";
			/* Only construct UI without start authentication */
			constructUI();
			switchToXScreensaver();
			screenState = XSCREENSAVER_BY_IDLE;
		}
		break;
	default:
		break;
	}
}



/*
 * Others
 */



QString MainWindow::getUserAvatarPath(QString username)
{
	QString iconPath;
	QDBusInterface userIface( "org.freedesktop.Accounts",
					"/org/freedesktop/Accounts",
					"org.freedesktop.Accounts",
					QDBusConnection::systemBus());
	if (!userIface.isValid())
		qDebug() << "userIface is invalid";
	QDBusReply<QDBusObjectPath> userReply = userIface.call("FindUserByName",
								username);
	if (!userReply.isValid()) {
		qDebug() << "userReply is invalid";
		iconPath = "/usr/share/kylin-greeter/default_face.png";
	}
	QDBusInterface iconIface( "org.freedesktop.Accounts",
					userReply.value().path(),
					"org.freedesktop.DBus.Properties",
					QDBusConnection::systemBus());
	if (!iconIface.isValid())
		qDebug() << "IconIface is invalid";
	QDBusReply<QDBusVariant> iconReply = iconIface.call("Get",
				"org.freedesktop.Accounts.User", "IconFile");
	if (!iconReply.isValid()) {
		qDebug() << "iconReply is invalid";
		iconPath = "/usr/share/kylin-greeter/default_face.png";
	}
	iconPath = iconReply.value().variant().toString();
	if (access(get_char_pointer(iconPath), R_OK) != 0) /* No Access Permission */
		qDebug() << "Can't access user avatar:" << iconPath
						<< "No access permission.";
	return iconPath;
}
