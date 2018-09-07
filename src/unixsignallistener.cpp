#include "unixsignallistener.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <QDebug>

int UnixSignalListener::sigusr1Fd[2];

UnixSignalListener::UnixSignalListener(QObject *parent) : QObject(parent)
{
	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigusr1Fd))
		qFatal("Couldn't create SIGUSR1 socketpair");
	snUsr1 = new QSocketNotifier(sigusr1Fd[1], QSocketNotifier::Read, this);
	connect(snUsr1, &QSocketNotifier::activated, this, &UnixSignalListener::handleSigUsr1);
}

void UnixSignalListener::usr1SignalAction(int sig, siginfo_t *siginfo, void *ucontext)
{
	(void)sig;
	(void)ucontext;
	char buffer[16];
	sprintf(buffer, "%d", siginfo->si_pid);
	int ignore = ::write(sigusr1Fd[0], buffer, strlen(buffer) + 1);
	(void)ignore;
}

void UnixSignalListener::chldSignalAction(int /*sig*/, siginfo_t */*siginfo*/, void */*ucontext*/)
{
    ::waitpid(-1, NULL, 0);
}

void UnixSignalListener::handleSigUsr1()
{
	snUsr1->setEnabled(false);
	char buffer[16];
	int pid;
	int ignore = ::read(sigusr1Fd[1], buffer, sizeof(buffer));
	sscanf(buffer, "%d", &pid);
	(void)ignore;

	/* Do Qt stuff */
	Q_EMIT transition(pid);

	snUsr1->setEnabled(true);
}
