/* configuration.h
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#ifndef PAM_TALLY_H
#define PAM_TALLY_H

#include <QObject>

class PamTally : public QObject
{
    Q_OBJECT
private:
    explicit PamTally(QObject *parent = nullptr);

public:
    static PamTally *instance(QObject *parent = nullptr);
    int getDeny();
    unsigned long long getUnlockTime();
    unsigned long long getRootUnlockTime();

private:
    int deny;
    unsigned long long unlock_time;
    unsigned long long  root_unlock_time;
    static PamTally    *instance_;

    int parsePamConfig();
};

#endif // CONFIGURATION_H
