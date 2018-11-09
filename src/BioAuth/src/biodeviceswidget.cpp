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
#include "biodeviceswidget.h"
#include "ui_biodeviceswidget.h"
#include <QPushButton>

static QMap<int, QList<DeviceInfo>> getTestDevices();

BioDevicesWidget::BioDevicesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BioDevicesWidget),
    btnGroup(new QButtonGroup(this)),
    lwDevices(new QListWidget(this))
{
    ui->setupUi(this);

    connect(&bioDevices, &BioDevices::deviceCountChanged,
            this, &BioDevicesWidget::onDeviceCountChanged);
    devicesMap = bioDevices.getAllDevices();
//    QMap<int, QList<DeviceInfo>> devicesMap = getTestDevices();
//    setDevices(devicesMap);

    ui->cmbDeviceName->setModel(lwDevices->model());
    ui->cmbDeviceName->setView(lwDevices);
}

BioDevicesWidget::~BioDevicesWidget()
{
    delete ui;
}

void BioDevicesWidget::setDevices(const QMap<int, QList<DeviceInfo> > &devices)
{
    for(int type : devices.keys())
    {
//        devicesMap[type].append(devices[type]);
    }
}

void BioDevicesWidget::init(uid_t uid)
{
    this->uid = uid;
//    devicesMap = bioDevices.getAllDevices();

    ui->cmbDeviceName->clear();

    addTypeButtons();

    /* set the default device as current device */
    DeviceInfo *device = bioDevices.getDefaultDevice(uid);
    if(!device)
    {
        device = bioDevices.getFirstDevice();
    }
    if(device)
    {
        QPushButton *btn = dynamic_cast<QPushButton*>(btnGroup->button(device->biotype));
        if(btn)
        {
            btn->click();
        }
    }
}

void BioDevicesWidget::addTypeButtons()
{
    for(auto btn : btnGroup->buttons())
    {
        btnGroup->removeButton(btn);
        btn->deleteLater();
    }

    const int w = 100;
    const int h = 26;
    const int xStart = ui->lblDeviceType->geometry().right() + 10;
    const int yStart = ui->lblDeviceType->y();
    int xPos, yPos;
    int i = 0;
    for(auto type : devicesMap.keys())
    {

        QString bioType = bioTypeToString(type);
        QString text = BioDevices::bioTypeToString_tr(type);
        QString iconName = QString(":/images/images/%1-icon.png").arg(bioType.toLower());
        QIcon icon(iconName);
        QPushButton *btn = new QPushButton(icon, text, this);
        btn->setObjectName("btn" + bioType);
        btn->show();
        btnGroup->addButton(btn, type);
        connect(btn, &QPushButton::clicked, this, &BioDevicesWidget::onDeviceTypeChanged);

        if(i < 3)
        {
            xPos = xStart + (100 + 10) * i;
            yPos = yStart;
        }
        else
        {
            xPos = xStart + (100 + 10) * (i - 3);
            yPos = yStart + h + 10;
        }
        btn->setGeometry(xPos, yPos, w, h);
        i++;
    }

    ui->lblDeviceName->move(ui->lblDeviceName->x(), yPos + h + 20);
    ui->cmbDeviceName->move(ui->cmbDeviceName->x(), yPos + h + 20);
    ui->btnOK->move(ui->btnOK->x(), ui->cmbDeviceName->geometry().bottom() + 20);
    ui->btnBack->move(ui->btnBack->x(), ui->cmbDeviceName->geometry().bottom() + 20);
}

void BioDevicesWidget::onDeviceTypeChanged()
{
    this->type = (BioType)btnGroup->id(dynamic_cast<QPushButton*>(sender()));
    changeButtonStyle();

    ui->cmbDeviceName->clear();
    lwDevices->clear();

    QList<DeviceInfo*> &deviceList = devicesMap[type];
    for(auto deviceInfo : deviceList)
    {
        lwDevices->addItem(deviceInfo->device_shortname);
    }

}


void BioDevicesWidget::onDeviceCountChanged()
{
//    int type = ui->cmbDeviceTypes->itemData(ui->cmbDeviceTypes->currentIndex()).toInt();
//    init(uid);
//    int index = ui->cmbDeviceTypes->findData(type);
//    ui->cmbDeviceTypes->setCurrentIndex(index >= 0 ? index : 0);

//    Q_EMIT deviceCountChanged(bioDevices.count());
}

void BioDevicesWidget::changeButtonStyle()
{
    QPushButton *btnActivate = dynamic_cast<QPushButton*>(btnGroup->button(type));
    btnActivate->setStyleSheet("QPushButton{background: rgba(0, 0, 0, 50);}");

    for(auto btn : btnGroup->buttons())
    {
        if(btn != btnActivate)
        {
            btn->setStyleSheet("QPushButton{background: rgba(0, 0, 0, 10)}");
        }
    }
}

void BioDevicesWidget::on_btnOK_clicked()
{
    int index = ui->cmbDeviceName->currentIndex();
    DeviceInfo *device = devicesMap[type].at(index);
    Q_EMIT deviceChanged(device);
    Q_EMIT back();
}

void BioDevicesWidget::on_btnBack_clicked()
{
    Q_EMIT back();
}

static QMap<int, QList<DeviceInfo>> getTestDevices()
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

