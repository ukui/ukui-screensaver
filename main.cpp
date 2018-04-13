#include "mainwindow.h"
#include <QApplication>
#include "unixsignallistener.h"
#include <signal.h>

static int setup_unix_signal_handlers();

int main(int argc, char *argv[])
{
	setup_unix_signal_handlers();
	QApplication a(argc, argv);
	UnixSignalListener unixSignalListener;
	MainWindow w;
	QObject::connect(&unixSignalListener, &UnixSignalListener::transition,
			&w, &MainWindow::FSMTransition);

	return a.exec();
}

static int setup_unix_signal_handlers()
{
    struct sigaction usr1;

    usr1.sa_handler = UnixSignalListener::usr1SignalHandler;
    sigemptyset(&usr1.sa_mask);
    usr1.sa_flags = 0;
    usr1.sa_flags |= SA_RESTART;

    if (sigaction(SIGUSR1, &usr1, 0))
       return 1;

    return 0;
}
