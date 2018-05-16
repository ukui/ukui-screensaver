#ifndef BIODEVICES_H
#define BIODEVICES_H

#include <QObject>
#include <QMap>
#include <QList>
#include "biocustomtype.h"

class QDBusInterface;

/*!
 * \brief The BioDevices class
 * this class saves the list of all device information
 * and list of available devices' info list for each uid
 */
class BioDevices : public QObject
{
    Q_OBJECT
public:
    explicit BioDevices(QObject *parent = nullptr);
    int deviceCount();
    int featuresNum(qint32 uid);
    QList<DeviceInfo> getAvaliableDevices(qint32 uid);
    void clear();

private:
    void connectToService();
    void getDevicesList();
    void getFeaturesList(qint32 uid);

private:
    QDBusInterface                          *serviceInterface;

    static bool                             isFirst;
    static QList<DeviceInfo*>               deviceInfos;        //the list of al device info
    static QMap<int, QList<DeviceInfo>>     savedDeviceInfos;   //[uid, avaliable DeviceInfos]
};



#endif // BIODEVICES_H
