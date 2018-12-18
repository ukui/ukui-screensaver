#include "biometricdeviceinfo.h"

#include <QDebug>
#include <QtDBus>
//#include <QDBusAbstractInterface>

QString DeviceType::getDeviceType(int deviceType)
{
    if(deviceType >= __MAX_NR_TYPES)
    {
        return "";
    }
    QMetaEnum meta = QMetaEnum::fromType<Type>();
    const char *typeString = meta.valueToKey(deviceType);
    return QString(typeString);
}

QString DeviceType::getDeviceType_tr(int deviceType)
{
    switch(deviceType)
    {
    case FingerPrint:
        return tr("FingerPrint");
    case FingerVein:
        return tr("FingerVein");
    case Iris:
        return tr("Iris");
    case Face:
        return tr("Face");
    case VoicePrint:
        return tr("VoicePrint");
    default:
        return "";
    }
}

QDebug operator <<(QDebug stream, const DeviceInfo &deviceInfo)
{
    stream << "["
           << deviceInfo.id
           << deviceInfo.shortName
           << deviceInfo.fullName
           << deviceInfo.deviceType
           << deviceInfo.driverEnable
           << deviceInfo.deviceNum
           << "]";
    return stream;
}

QDBusArgument &operator <<(QDBusArgument &arg, const DeviceInfo &deviceInfo)
{
    arg.beginStructure();
    arg << deviceInfo.id
        << deviceInfo.shortName
        << deviceInfo.fullName
        << deviceInfo.driverEnable
        << deviceInfo.deviceNum
        << deviceInfo.deviceType
        << deviceInfo.storageType
        << deviceInfo.eigType
        << deviceInfo.verifyType
        << deviceInfo.identifyType
        << deviceInfo.busType
        << deviceInfo.deviceStatus
        << deviceInfo.OpsStatus;
    arg.endStructure();
    return arg;
}
const QDBusArgument &operator >>(const QDBusArgument &arg, DeviceInfo &deviceInfo)
{
    arg.beginStructure();
    arg >> deviceInfo.id
        >> deviceInfo.shortName
        >> deviceInfo.fullName
        >> deviceInfo.driverEnable
        >> deviceInfo.deviceNum
        >> deviceInfo.deviceType
        >> deviceInfo.storageType
        >> deviceInfo.eigType
        >> deviceInfo.verifyType
        >> deviceInfo.identifyType
        >> deviceInfo.busType
        >> deviceInfo.deviceStatus
        >> deviceInfo.OpsStatus;
    arg.endStructure();
    return arg;
}

void registerMetaType()
{
    qRegisterMetaType<DeviceInfo>("DeviceInfo");
    qDBusRegisterMetaType<DeviceInfo>();
}
