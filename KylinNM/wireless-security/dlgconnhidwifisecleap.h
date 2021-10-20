/*
 * Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef DLGCONNHIDWIFISECLEAP_H
#define DLGCONNHIDWIFISECLEAP_H

#include <QDialog>
#include <QMouseEvent>

namespace Ui {
class DlgConnHidWifiSecLeap;
}

class DlgConnHidWifiSecLeap : public QDialog
{
    Q_OBJECT

public:
    explicit DlgConnHidWifiSecLeap(int type, QWidget *parent = 0);
    ~DlgConnHidWifiSecLeap();

protected:
    void paintEvent(QPaintEvent *event);

    /**解决锁屏设置 X11BypassWindowManagerHint 属性导致QCombox弹框异常的问题---START----
    ** 手动绑定下拉框视图和下拉框 过滤点击事件
    **/
    bool eventFilter(QObject *obj, QEvent *ev) override;

public slots:
    void changeDialogSecu();
    void changeDialogAuth();

private slots:
    void on_btnCancel_clicked();

    void on_btnConnect_clicked();

    void on_checkBox_stateChanged(int arg1);

    void on_leNetName_textEdited(const QString &arg1);

    void on_leUserName_textEdited(const QString &arg1);

    void on_lePassword_textEdited(const QString &arg1);

private:
    Ui::DlgConnHidWifiSecLeap *ui;
    int WepOrWpa = 0;//0 WEP;1WPA

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

//    QString labelQss, cbxQss, leQss, btnConnQss, btnCancelQss, lineQss, checkBoxQss, checkBoxCAQss;

    bool isPress;
    QPoint winPos;
    QPoint dragPos;
};

#endif // DLGCONNHIDWIFISECLEAP_H
