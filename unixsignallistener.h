#ifndef UNIXSIGNALLISTENER_H
#define UNIXSIGNALLISTENER_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QObject>
#include <QSocketNotifier>

class UnixSignalListener : public QObject
{
	Q_OBJECT
public:
	explicit UnixSignalListener(QObject *parent = nullptr);

public:
	/* Unix signal handlers. */
	static void usr1SignalHandler(int unused);

Q_SIGNALS:
	void transition(); /* Finite State Machine Driven Signal */

public Q_SLOTS:
	/* Qt signal handlers. */
	void handleSigUsr1();

private:
	static int sigusr1Fd[2];
	QSocketNotifier *snUsr1;
};

#endif // UNIXSIGNALLISTENER_H
