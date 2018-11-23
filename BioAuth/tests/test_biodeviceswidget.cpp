#include <QApplication>

#include "biodeviceswidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    BioDevicesWidget w;
    QMap<int, QList<DeviceInfo*>> devicesMap = getTestDevices();
    w.setDevices(devicesMap);
    w.init(1000);
    w.show();

    return app.exec();
}
