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
#include "grab-x11.h"
#include <QX11Info>
#include <X11/Xlib.h>
#include <xcb/xcb.h>

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
