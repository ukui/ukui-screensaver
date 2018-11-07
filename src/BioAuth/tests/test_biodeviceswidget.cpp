#include <QApplication>

#include "biodeviceswidget.h"

QMap<int, QList<DeviceInfo>> getTestDevices()
{
    QMap<int, QList<DeviceInfo>> devicesMap;
    QList<DeviceInfo> fpList, feList, iiList, fcList, vpList;
    DeviceInfo fpDevice1, feDevice1, iiDevice1, fcDevice1, vpDevice1;

    fpDevice1.biotype = BIOTYPE_FINGERPRINT;
    fpDevice1.device_shortname = "fpDevice1";
    fpList.push_back(fpDevice1);
    devicesMap[BIOTYPE_FINGERPRINT] = fpList;

    feDevice1.biotype = BIOTYPE_FINGERVEIN;
    feDevice1.device_shortname = "feDevice1";
    feList.push_back(feDevice1);
    devicesMap[BIOTYPE_FINGERVEIN] = feList;

    iiDevice1.biotype = BIOTYPE_IRIS;
    iiDevice1.device_shortname = "iiDevice1";
    iiList.push_back(iiDevice1);
    devicesMap[BIOTYPE_IRIS] = iiList;

    fcDevice1.biotype = BIOTYPE_FACE;
    fcDevice1.device_shortname = "fcDevice1";
    fcList.push_back(fcDevice1);
    devicesMap[BIOTYPE_FACE] = fcList;

    vpDevice1.biotype = BIOTYPE_VOICEPRINT;
    vpDevice1.device_shortname = "vpDevice1";
    vpList.push_back(vpDevice1);
    devicesMap[BIOTYPE_VOICEPRINT] = vpList;

    return devicesMap;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    BioDevicesWidget w;
    QMap<int, QList<DeviceInfo>> devicesMap = getTestDevices();
    w.setDevices(devicesMap);
    w.init(1000);
    w.show();

    return app.exec();
}
