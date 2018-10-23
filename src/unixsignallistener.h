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
#ifndef UNIXSIGNALLISTENER_H
#define UNIXSIGNALLISTENER_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QObject>
#include <QSocketNotifier>
#include <signal.h>

class UnixSignalListener : public QObject
{
	Q_OBJECT
public:
	explicit UnixSignalListener(QObject *parent = nullptr);

public:
	/* Unix signal handlers. */
	static void usr1SignalAction(int sig, siginfo_t *siginfo, void *ucontext);
    static void chldSignalAction(int sig, siginfo_t *siginfo, void *ucontext);

Q_SIGNALS:
	/* pid is the sending process id */
	void transition(int pid); /* Finite State Machine Driven Signal */

public Q_SLOTS:
	/* Qt signal handlers. */
	void handleSigUsr1();

private:
	static int sigusr1Fd[2];
	QSocketNotifier *snUsr1;
};

#endif // UNIXSIGNALLISTENER_H
