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
#ifndef TYPES_H
#define TYPES_H

/* https://www.narf.ssji.net/~shtrom/wiki/projets/gnomescreensavernosession */
enum SessionStatus
{
    SESSION_AVAILABLE = 0,
    SESSION_INVISIBLE = 1,
    SESSION_BUSY = 2,
    SESSION_IDLE = 3
};

enum ScreenStatus
{
    UNDEFINED = 0x00,
    SCREEN_SAVER = 0x01,
    SCREEN_LOCK = 0x02
};

#define SM_DBUS_SERVICE     "org.gnome.SessionManager"
#define SM_DBUS_PATH        "/org/gnome/SessionManager/Presence"
#define SM_DBUS_INTERFACE   "org.gnome.SessionManager.Presence"

#define BIO_ERROR -1
#define BIO_FAILED 0
#define BIO_SUCCESS 1
#define BIO_IGNORE 2

#define BIOMETRIC_PAM       "BIOMETRIC_PAM"
#define BIOMETRIC_IGNORE    "BIOMETRIC_IGNORE"
#define BIOMETRIC_SUCCESS   "BIOMETRIC_SUCCESS"
#define BIOMETRIC_FAILED    "BIOMETRIC_FAILED"

#endif // TYPES_H
