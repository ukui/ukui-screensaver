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
#include "util.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

char *get_char_pointer(QString string)
{
	char *buffer;
	buffer = (char *)malloc(string.length() + 1);
	strcpy(buffer, string.toLocal8Bit().data());
	return buffer;
}



bool checkCapsLockState()
{
    //判断大写键状态
    Display *display = XOpenDisplay(NULL);
    bool capsState = false;
    if(display) {
        unsigned int n;
        XkbGetIndicatorState(display, XkbUseCoreKbd, &n);
        capsState = (n & 0x01) == 1;
    }
    return capsState;
}
