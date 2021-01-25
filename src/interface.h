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
#ifndef INTERFACE_H
#define INTERFACE_H

#include <QObject>
#include <QDBusContext>
#include <QProcess>
#include <QTimer>
#include <QDBusUnixFileDescriptor>
#include "types.h"
#include "logind.h"

class QGSettings;
class Interface : public QObject, protected QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", SS_DBUS_SERVICE)

public:
    explicit Interface(QObject *parent = nullptr);
	 LogindIntegration *m_logind;
Q_SIGNALS:
    void SessionIdle();

public Q_SLOTS:
    /**
     * Lock the screen
     */
    void Lock();
    bool GetLockState();
    void SetLockState();
    void onSessionIdleReceived();
    void onShowScreensaver();
    void onShowBlankScreensaver();
    void onNameLost(const QString&);
    void onPrepareForSleep(bool sleep);

private:
    bool checkExistChild();
    void inhibit();
    void uninhibit();
    bool lockState;
    void emitLockState(bool);
    int m_timerCount;
    QTimer *m_timer;
    QDBusUnixFileDescriptor m_inhibitFileDescriptor;
    QGSettings	*settings;

private:
    QProcess process;
};

#endif // INTERFACE_H
