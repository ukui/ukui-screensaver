/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
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
