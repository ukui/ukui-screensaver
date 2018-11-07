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
#ifndef BIOAUTHWIDGET_H
#define BIOAUTHWIDGET_H

#include <QWidget>
#include "bioauth.h"

namespace Ui {
class BioAuthWidget;
}

class BioAuthWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BioAuthWidget(QWidget *parent = 0);
    ~BioAuthWidget();

public Q_SLOTS:
    void startAuth(uid_t uid, const DeviceInfo &device);
    void setMoreDevices(bool hasMore);

Q_SIGNALS:
    void switchToPassword();
    void selectDevice();
    void authComplete(uid_t uid, bool ret);

private slots:
    void on_btnPasswdAuth_clicked();
    void on_btnMore_clicked();
    void on_btnRetry_clicked();

    void onBioAuthNotify(const QString &notifyMsg);
    void onBioAuthComplete(uid_t uid, bool ret);

private:
    void setMovie();
    void setImage();

private:
    Ui::BioAuthWidget *ui;
    BioAuth *bioAuth;
    uid_t uid;
    DeviceInfo device;
};

#endif // BIOAUTHWIDGET_H
