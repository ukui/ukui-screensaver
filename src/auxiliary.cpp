#include "auxiliary.h"
#include <QX11Info>
#include <X11/Xlib.h>
#include <xcb/xcb.h>

char *get_char_pointer(QString string)
{
	char *buffer;
	buffer = (char *)malloc(string.length() + 1);
	strcpy(buffer, string.toLocal8Bit().data());
	return buffer;
}


class XServerGraber{
public:
    XServerGraber() {
        xcb_grab_server(QX11Info::connection());
    }
    ~XServerGraber() {
        xcb_ungrab_server(QX11Info::connection());
        xcb_flush(QX11Info::connection());
    }
};

static bool grabKeyboard()
{
    int rv = XGrabKeyboard(QX11Info::display(), QX11Info::appRootWindow(),
                           True, GrabModeAsync, GrabModeAsync, CurrentTime);
    return (rv == GrabSuccess);
}

static bool grabMouse()
{
#define GRABEVENTS ButtonPressMask | ButtonReleaseMask | PointerMotionMask | \
                   EnterWindowMask | LeaveWindowMask
    int rv = XGrabPointer(QX11Info::display(), QX11Info::appRootWindow(),
                          True, GRABEVENTS, GrabModeAsync, GrabModeAsync, None,
                          None, CurrentTime);
#undef GRABEVENTS

    return (rv == GrabSuccess);
}

bool establishGrab()
{
    XSync(QX11Info::display(), False);
    XServerGraber xserverGraber;

    Q_UNUSED(xserverGraber);

    if(!grabKeyboard())
        return false;

    if(!grabMouse()) {
        XUngrabKeyboard(QX11Info::display(), CurrentTime);
        XFlush(QX11Info::display());
        return false;
    }

    return true;
}

bool closeGrab()
{
    XSync(QX11Info::display(), False);
    XServerGraber xserverGraber;

    Q_UNUSED(xserverGraber);

    XUngrabKeyboard(QX11Info::display(), CurrentTime);
    XUngrabPointer(QX11Info::display(), CurrentTime);
    XFlush(QX11Info::display());
    return true;
}
