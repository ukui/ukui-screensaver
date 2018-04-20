#include "unixsignallistener.h"
#include <sys/socket.h>
#include <unistd.h>
#include <QDebug>

int UnixSignalListener::sigusr1Fd[2];

UnixSignalListener::UnixSignalListener(QObject *parent) : QObject(parent)
{
	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigusr1Fd))
		qFatal("Couldn't create SIGUSR1 socketpair");
	snUsr1 = new QSocketNotifier(sigusr1Fd[1], QSocketNotifier::Read, this);
	connect(snUsr1, &QSocketNotifier::activated, this, &UnixSignalListener::handleSigUsr1);
}

void UnixSignalListener::usr1SignalHandler(int unused)
{
	(void)unused;
	char a = 1;
	int ignore = ::write(sigusr1Fd[0], &a, sizeof(a));
	(void)ignore;
}

void UnixSignalListener::handleSigUsr1()
{
	snUsr1->setEnabled(false);
	char tmp;
	int ignore = ::read(sigusr1Fd[1], &tmp, sizeof(tmp));
	(void)ignore;

	/* Do Qt stuff */
	emit transition();

	snUsr1->setEnabled(true);
}
