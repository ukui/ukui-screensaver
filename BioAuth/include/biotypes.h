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
#ifndef BIOCUSTOMTYPE_H
#define BIOCUSTOMTYPE_H

#include <QObject>
#include <QtDBus>

#define BIO_DBUS_SERVICE "org.ukui.Biometric"
#define BIO_DBUS_PATH "/org/ukui/Biometric"
#define BIO_DBUS_INTERFACE "org.ukui.Biometric"


#define DEFAULT_DEVICE "DefaultDevice"


#ifdef LOG
#undef LOG
#endif
#define LOG() qDebug() << "[BIOMETRIC]"

/* the type of device */
enum BioType {
    BIOTYPE_FINGERPRINT,
    BIOTYPE_FINGERVEIN,
    BIOTYPE_IRIS,
    BIOTYPE_FACE,
    BIOTYPE_VOICEPRINT,
    __MAX_NR_BIOTYPES
};

/* StatusChanged D-Bus 信号触发时的状态变化类型 */
enum StatusType {
    STATUS_DEVICE,
    STATUS_OPERATION,
    STATUS_NOTIFY
};

/* 录入/删除/搜索等 D-Bus 调用的最终结果，即返回值里的 result */
enum DBusResult {
    DBUS_RESULT_SUCCESS = 0,
    DBUS_RESULT_ERROR,
    DBUS_RESULT_DEVICEBUSY,
    DBUS_RESULT_NOSUCHDEVICE,
    DBUS_RESULT_PERMISSIONDENIED
};


/* the info of device */
struct DeviceInfo {
    int device_id;
    QString device_shortname; /* aka driverName */
    QString device_fullname;
    int driver_enable; /* The corresponding driver is enabled/disabled */
    int device_available; /* The driver is enabled and the device is connected */
    int biotype;
    int stotype;
    int eigtype;
    int vertype;
    int idtype;
    int bustype;
    int dev_status;
    int ops_status;

    bool operator==(const DeviceInfo&) const;

    bool operator !=(const DeviceInfo &deviceInfo) const
    {
        return !(*this == deviceInfo);
    }
    friend QDebug& operator<<(QDebug &stream, const DeviceInfo &deviceInfo);

};
Q_DECLARE_METATYPE(DeviceInfo)

QDBusArgument &operator<<(QDBusArgument &argument, const DeviceInfo &deviceInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, DeviceInfo &deviceInfo);

QString bioTypeToString(int type);


#endif // BIOCUSTOMTYPE_H
