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
#include "biotypes.h"
#include <QDBusInterface>


QDBusArgument &operator<<(QDBusArgument &argument, const DeviceInfo &deviceInfo)
{
    argument.beginStructure();
    argument << deviceInfo.device_id << deviceInfo.device_shortname
        << deviceInfo.device_fullname << deviceInfo.driver_enable
        << deviceInfo.device_available
        << deviceInfo.biotype << deviceInfo.stotype
        << deviceInfo.eigtype << deviceInfo.vertype
        << deviceInfo.idtype << deviceInfo.bustype
        << deviceInfo.dev_status << deviceInfo.ops_status;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DeviceInfo &deviceInfo)
{
    argument.beginStructure();
    argument >> deviceInfo.device_id >> deviceInfo.device_shortname
        >> deviceInfo.device_fullname >> deviceInfo.driver_enable
        >> deviceInfo.device_available
        >> deviceInfo.biotype >> deviceInfo.stotype
        >> deviceInfo.eigtype >> deviceInfo.vertype
        >> deviceInfo.idtype >> deviceInfo.bustype
        >> deviceInfo.dev_status >> deviceInfo.ops_status;
    argument.endStructure();
    return argument;
}

bool DeviceInfo::operator==(const DeviceInfo& deviceInfo) const
{
    if(this->device_shortname == deviceInfo.device_shortname &&
       this->device_available == deviceInfo.device_available)
        return true;
    return false;
}

QDebug& operator<<(QDebug &stream, const DeviceInfo &deviceInfo)
{
    stream << deviceInfo.device_id << deviceInfo.device_shortname
           << deviceInfo.biotype << deviceInfo.device_available;
    return stream;
}

QString bioTypeToString(int type)
{
    switch(type) {
    case BIOTYPE_FINGERPRINT:
        return ("FingerPrint");
    case BIOTYPE_FINGERVEIN:
        return ("FingerVein");
    case BIOTYPE_IRIS:
        return ("Iris");
    case BIOTYPE_FACE:
        return ("Face");
    case BIOTYPE_VOICEPRINT:
        return ("VoicePrint");
    }
    return QString();
}
