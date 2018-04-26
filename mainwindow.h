#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <gsettings.h>
#include <QMainWindow>
#include <QtDBus/QDBusInterface>
#include "configuration.h"

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

private:
	void setRealTimeMouseTracking();
	void embedXScreensaver();
	void handleKeyPressEvent(QKeyEvent *event);
	void handleMouseMoveEvent(QMouseEvent *event);
	void switchToLockscreen();
	void switchToXScreensaver();
	void startXScreensaverWithoutAuth();
	bool eventFilter(QObject *watched, QEvent *event);
	void constructUI();
	void closeEvent(QCloseEvent *event);
	void uiGetReady(bool ready);
	void setWindowStyle();
	void lockscreenFollowCursor(QPoint cursorPosition);
	QString getUserAvatarPath(QString username);

protected:
	void paintEvent(QPaintEvent *event);

public Q_SLOTS:
	void FSMTransition(); /* Transition FSM states according to signal */

private Q_SLOTS:
	void onUnlockClicked();
	void onPasswordEnter();
	void sessionStatusChanged(unsigned int status);

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
};

#endif // MAINWINDOW_H
