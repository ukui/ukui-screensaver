/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2017 Deepin, Inc.
 *               2011 ~ 2017 Wang Yong
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "xeventmonitor.h"
#include <iostream>
#include <X11/Xlibint.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>

// Virtual button codes that are not defined by X11.
#define Button1            1
#define Button2            2
#define Button3            3
#define WheelUp            4
#define WheelDown        5
#define WheelLeft        6
#define WheelRight        7
#define XButton1        8
#define XButton2        9

class XEventMonitorPrivate
{
public:
    XEventMonitorPrivate(XEventMonitor *parent);
    virtual ~XEventMonitorPrivate();
    void run();

protected:
    XEventMonitor *q_ptr;

    bool filterWheelEvent(int detail);
    static void callback(XPointer trash, XRecordInterceptData* data);
    void handleRecordEvent(XRecordInterceptData *);
    void emitButtonSignal(const char *member, xEvent *event);
    void emitKeySignal(const char *member, xEvent *event);

private:
    Q_DECLARE_PUBLIC(XEventMonitor)
};

XEventMonitorPrivate::XEventMonitorPrivate(XEventMonitor *parent)
    : q_ptr(parent)
{

}

XEventMonitorPrivate::~XEventMonitorPrivate()
{

}

void XEventMonitorPrivate::emitButtonSignal(const char *member, xEvent *event)
{
    int x = event->u.keyButtonPointer.rootX;
    int y = event->u.keyButtonPointer.rootY;
    QMetaObject::invokeMethod(q_ptr, member,
                              Qt::DirectConnection,
                              Q_ARG(int, x),
                              Q_ARG(int, y));
}

void XEventMonitorPrivate::emitKeySignal(const char *member, xEvent *event)
{
    Display *display = XOpenDisplay(NULL);
    int keyCode = event->u.u.detail;
    KeySym keySym = XkbKeycodeToKeysym(display, event->u.u.detail, 0, 0);
    char *keyStr = XKeysymToString(keySym);
    QMetaObject::invokeMethod(q_ptr, member,
                              Qt::AutoConnection,
                              Q_ARG(int, keyCode));
    QMetaObject::invokeMethod(q_ptr, member,
                              Qt::AutoConnection,
                              Q_ARG(QString, keyStr));
    XCloseDisplay(display);
}

void XEventMonitorPrivate::run()
{
    Display* display = XOpenDisplay(0);
    if (display == 0) {
        fprintf(stderr, "unable to open display\n");
        return;
    }

    // Receive from ALL clients, including future clients.
    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange* range = XRecordAllocRange();
    if (range == 0) {
        fprintf(stderr, "unable to allocate XRecordRange\n");
        return;
    }

    // Receive KeyPress, KeyRelease, ButtonPress, ButtonRelease and MotionNotify events.
    memset(range, 0, sizeof(XRecordRange));
    range->device_events.first = KeyPress;
    range->device_events.last  = MotionNotify;

    // And create the XRECORD context.
    XRecordContext context = XRecordCreateContext(display, 0, &clients, 1, &range, 1);
    if (context == 0) {
        fprintf(stderr, "XRecordCreateContext failed\n");
        return;
    }
    XFree(range);

    XSync(display, True);

    Display* display_datalink = XOpenDisplay(0);
    if (display_datalink == 0) {
        fprintf(stderr, "unable to open second display\n");
        return;
    }
    if (!XRecordEnableContext(display_datalink, context,  callback, (XPointer) this)) {
        fprintf(stderr, "XRecordEnableContext() failed\n");
        return;
    }

}

void XEventMonitorPrivate::callback(XPointer ptr, XRecordInterceptData* data)
{
    ((XEventMonitorPrivate *) ptr)->handleRecordEvent(data);
}

void XEventMonitorPrivate::handleRecordEvent(XRecordInterceptData* data)
{

    if (data->category == XRecordFromServer) {
        xEvent * event = (xEvent *)data->data;
        switch (event->u.u.type)
        {
        case ButtonPress:
            if (filterWheelEvent(event->u.u.detail)) {
                emitButtonSignal("buttonPress", event);
            }
            break;
        case MotionNotify:
            emitButtonSignal("buttonDrag", event);
            break;
        case ButtonRelease:
            if (filterWheelEvent(event->u.u.detail)) {
                emitButtonSignal("buttonRelease", event);
            }
            break;
        case KeyPress:
            emitKeySignal("keyPress", event);
            break;
        case KeyRelease:
            emitKeySignal("keyRelease", event);
            break;
        default:
            break;
        }
    }
    fflush(stdout);
    XRecordFreeData(data);
}

bool XEventMonitorPrivate::filterWheelEvent(int detail)
{
    return detail != WheelUp && detail != WheelDown && detail != WheelLeft && detail != WheelRight;
}


XEventMonitor::XEventMonitor(QObject *parent)
    : QThread(parent),
      d_ptr(new XEventMonitorPrivate(this))
{
    Q_D(XEventMonitor);
}

XEventMonitor::~XEventMonitor()
{
    requestInterruption();
    quit();
    wait();
}

void XEventMonitor::run()
{
    if(!isInterruptionRequested())
    {
        d_ptr->run();
    }
}
