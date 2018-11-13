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
#ifndef BIODEVICESWIDGET_H
#define BIODEVICESWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QListWidget>
#include "biodevices.h"

namespace Ui {
class BioDevicesWidget;
}

QMap<int, QList<DeviceInfo*>> getTestDevices();

class BioDevicesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BioDevicesWidget(QWidget *parent = 0);
    ~BioDevicesWidget();
    void init(uid_t uid);
    void setDevices(const QMap<int, QList<DeviceInfo *> > &devices);
    DeviceInfo *getSelectedDevice();

private:
    void addTypeButtons();
    void changeButtonStyle();

Q_SIGNALS:
    void deviceCountChanged(int count);

private Q_SLOTS:
    void onDeviceTypeChanged();
    void onDeviceCountChanged();

private:
    Ui::BioDevicesWidget *ui;
    BioDevices bioDevices;
    QMap<int, QList<DeviceInfo*>> devicesMap;
    QButtonGroup *btnGroup;
    QListWidget *lwDevices;
    uid_t uid;
    BioType type;
};

#endif // BIODEVICESWIDGET_H
