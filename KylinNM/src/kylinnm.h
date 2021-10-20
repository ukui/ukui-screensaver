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

#ifndef KYLINNM_H
#define KYLINNM_H

#include "ksimplenm.h"
#include "loadingdiv.h"
#include "confform.h"
#include "kylin-dbus-interface.h"
#include "kylin-network-interface.h"
#include "utils.h"
#include "switchbutton.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <xcb/xcb.h>

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QDesktopWidget>
#include <QScrollBar>
#include <QScrollArea>
#include <QPushButton>
#include <QScreen>
#include <QMenu>
#include <QAction>
#include <QAction>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QString>
#include <QTimer>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QDBusObjectPath>
#include <QVariant>
#include <QStandardPaths>
#include <QStringList>
#include <QToolTip>

#include "oneconnform.h"
#include "onelancform.h"
#include "hot-spot/dlghotspotcreate.h"
#include "wireless-security/dlgconnhidwifi.h"

#define W_LEFT_AREA 16
//#define W_VERTICAL_LINE 1 //左边竖线宽度
#define W_RIGHT_AREA 438 //41 + 1 + 438 = 480
#define L_VERTICAL_LINE_TO_ITEM 4 //竖线到item左侧的距离

#define X_LEFT_WIFI_BALL 416 //白色小球在左边
#define X_RIGHT_WIFI_BALL 440 //白色小球在右边
#define Y_WIFI_BALL 23 //白色小球y坐标
#define X_ITEM 46 //item到窗口左侧的距离 41 + 1 + 4 = 46
#define W_ITEM 424

#define Y_TOP_ITEM 100 //顶部item、topLanListWidget、topWifiListWidget的y坐标
#define H_NORMAL_ITEM 56
#define H_GAP_UP -5
#define H_MIDDLE_WORD 46 //"显示‘可用网络列表’的label"
#define H_GAP_DOWN 5 //57 + 60 + 10 + 46 + 5 = 178
#define X_MIDDLE_WORD 0
#define W_MIDDLE_WORD 260

#define H_LAN_ITEM_EXTEND 102 //162 - 60
#define H_WIFI_ITEM_BIG_EXTEND 90 //150 - 60
#define H_WIFI_ITEM_SMALL_EXTEND 100 //156 - 56

#define Y_SCROLL_AREA 192//列表纵坐标
#define W_SCROLL_AREA 414
//#define H_SCROLL_AREA 200
#define H_SCROLL_AREA H_NORMAL_ITEM*4 //平板中去掉了添加新网络按钮，设置为4倍的item,防止最后一栏被遮挡

#define W_TOP_LIST_WIDGET 435
#define W_LIST_WIDGET 414

#define W_BTN_FUN 128
#define H_BTN_FUN 24

#define X_BTN_FUN 36
#define Y_BTN_FUN 400 //新建网络，加入网络按钮的宽高、x坐标、y坐标

#define W_NO_ITEM_TIP 220
#define H_NO_ITEM_TIP 20

class OneConnForm;
class ConfForm;

namespace Ui {
class KylinNM;
}

class KylinNM : public QWidget
{
    Q_OBJECT

public slots:
    void onPhysicalCarrierChanged(bool flag);
    void onCarrierUpHandle();
    void onCarrierDownHandle();
    void onDeleteLan();
    void onNetworkDeviceAdded(QDBusObjectPath objPath);
    void onNetworkDeviceRemoved(QDBusObjectPath objPath);
    void getLanBandWidth();

    void onExternalConnectionChange(QString type);
    void onExternalLanChange();
    void onExternalWifiChange();
    void onExternalWifiSwitchChange(bool wifiEnabled);

    void oneLanFormSelected(QString lanName, QString uniqueName);
    void oneTopLanFormSelected(QString lanName, QString uniqueName);
    void oneWifiFormSelected(QString wifiName, int extendLength);
    void oneTopWifiFormSelected(QString wifiName, int extendLength);

    void on_btnHotspot_clicked();
    void on_btnHotspotState();

    //flag =0或1为普通点击、2为收到打开信息、3为收到关闭信息、4为无线网卡插入、5为无线网卡拔出
    void onBtnWifiClicked(int flag = 0);

    void connLanDone(int connFlag);
    void connWifiDone(int connFlag);

    void activeGetWifiList();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

public:
    explicit KylinNM(QWidget *parent = 0);
    ~KylinNM();

    void onSwipeGesture(int dx, int dy);
    void editQssString();
    void createTopLanUI();
    void createTopWifiUI();
    void createOtherUI();
    void createListAreaUI();
    void createLeftAreaUI();

    void startLoading();
    void stopLoading();

    void setTrayIcon(QIcon icon);
    void setTrayLoading(bool isLoading);
    void getActiveInfo();

    void initTimer();
    void checkIsWirelessDeviceOn();
    void updateNetList();
    void updateWifiList();

    int getConnectStatus();//获取网络连接状态，返回值 -1 无连接； 0 有线连接； 1无线连接； 2有线和无线都已连接

    QIcon iconLanOnline, iconLanOffline;
    QIcon iconWifiFull, iconWifiHigh, iconWifiMedium, iconWifiLow;
    QIcon iconConnecting;
    QList<QIcon> loadIcons;
    QString mwBandWidth;
    KylinDBus *objKyDBus = nullptr;
    NetworkSpeed *objNetSpeed = nullptr;
    SwitchButton *btnWireless;

    QStringList lanNameList;

    //状态设置,0为假，1为真
    int is_update_wifi_list = 0; //是否是update wifi列表，而不是load wifi列表
    int is_by_click_connect = 0; //是否是通过点击连接按钮进行的连接
    int is_btnNetList_clicked = 1; //是否处于有线网界面
    int is_btnWifiList_clicked = 0; //是否处于无线网界面
    int is_wired_line_ready = 1; //主机是否连接网线
    int is_wireless_adapter_ready = 1; //主机是否插入无线网卡
    int is_keep_wifi_turn_on_state = 1; //是否要执行wifi开关变为打开样式
    int is_stop_check_net_state = 0; //是否要在进行其他操作时停止检查网络状态
    int is_connect_wifi_failed = 0; //刚才是否连接wifi失败
    int is_fly_mode_on = 0; //是否已经打开飞行模式
    int is_hot_sopt_on = 0; //是否已经打开热点

    QString currSelNetName = ""; //当前ScrollArea中选中的网络名称
    int currSelNetNum = 0; //当前选中的item序号

    bool isTabletStyle=false;//YYF 平板桌面模式

    static void reflashWifiUi();
    static bool m_is_reflashWifiUi;
private:
    void tabletStyle();//YYF 平板桌面模式特有设置

    void checkSingle();
    void initNetwork();
    void createTrayIcon();
    void handleIconClicked();
    void showTrayIconMenu();
    bool checkLanOn();
    bool checkWlOn();
    void getLanList();
    void getWifiList();
    void getInitLanSlist();

    Ui::KylinNM *ui;

    LoadingDiv *loading = nullptr;

    QDesktopWidget desktop;
    KSimpleNM *ksnm = nullptr;
    ConfForm *confForm = nullptr;
    QWidget *topLanListWidget = nullptr;
    QWidget *topWifiListWidget = nullptr;
    QWidget *lanListWidget = nullptr;
    QWidget *wifiListWidget = nullptr;
    QWidget *optWifiWidget = nullptr;

    QLabel *lbLoadDown = nullptr;
    QLabel *lbLoadDownImg = nullptr;
    QLabel *lbLoadUp = nullptr;
    QLabel *lbLoadUpImg = nullptr;

    QLabel *lbNoItemTip = nullptr;
    bool ifLanConnected;
    bool ifWLanConnected;

    QScrollArea *scrollAreal = nullptr;
    QScrollArea *scrollAreaw = nullptr;
    QLabel *lbTopLanList = nullptr;
    QLabel *lbTopWifiList = nullptr;
    QLabel *lbLanList = nullptr;
    QLabel *lbWifiList = nullptr;
    QPushButton *btnAddNet = nullptr;
    QPushButton *btnCreateNet = nullptr;

    QSystemTrayIcon *trayIcon = nullptr;
    QMenu *trayIconMenu = nullptr;
    QAction *mShowWindow = nullptr;
    QAction *mAdvConf = nullptr;
    QWidget *widShowWindow = nullptr;
    QWidget *widAdvConf = nullptr;

    QString lname, wname; // 以太网卡和无线网卡名称

    QString btnOffQss, btnOnQss, btnBgOffQss, btnBgOnQss, btnBgHoverQss, btnBgLeaveQss; // 主界面按钮底色
    QString scrollBarQss, leftBtnQss, funcBtnQss;

    QStringList oldLanSlist; //上一次获取Lan列表

    QStringList oldWifiSlist; //上一次获取wifi列表

    //平板
    QLabel *lbNetListText=nullptr;//有线网络
    QLabel *lbWifiListText=nullptr;//无线网络


    //循环检测网络连接状态
    QTimer *iconTimer = nullptr;
    QTimer *wiredCableUpTimer = nullptr;
    QTimer *wiredCableDownTimer = nullptr;
    QTimer *deleteLanTimer = nullptr;
    QTimer *checkWifiListChanged = nullptr;
    QTimer *checkIfLanConnect = nullptr;
    QTimer *checkIfWifiConnect = nullptr;
    QTimer *checkIfNetworkOn = nullptr;
    QTimer *setNetSpeed = nullptr;

    int currentIconIndex;
    int activeWifiSignalLv;

    long int start_rcv_rates = 0;	//保存开始时的流量计数
    long int end_rcv_rates = 0;	//保存结束时的流量计数
    long int start_tx_rates = 0;   //保存开始时的流量计数
    long int end_tx_rates = 0; //保存结束时的流量计数

    QString actWifissid = "--";//当前连接wifi的ssid
    QString actWifiuuid = "--";//当前连接wifi的uuid
    QStringList actWifiBssidList; //当前连接wifi的bssid

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

    void on_btnNet_clicked();
    void on_btnWifiList_clicked();
    void onBtnNetListClicked(int flag=0);

    void getLanListDone(QStringList slist);
    void getWifiListDone(QStringList slist);
    void loadWifiListDone(QStringList slist);
    void updateWifiListDone(QStringList slist);

    void on_showWindowAction();
    void on_btnAdvConf_clicked();
    void on_btnNetList_pressed();
    void on_btnWifiList_pressed();

    void activeLanDisconn();
    void activeWifiDisconn();
    void activeStartLoading();
    void on_btnAdvConf_pressed();
    void on_btnAdvConf_released();
    void on_checkWifiListChanged();
    void on_setNetSpeed();
    void on_checkOverTime();

    // 后台回调
    void enNetDone();
    void disNetDone();
    void enWifiDone();
    void launchLanDone();
    void disWifiDone();
    void disWifiStateKeep();
    void disWifiDoneChangeUI();

    void iconStep();
    void on_btnFlyMode_clicked();

    void onBtnAddNetClicked();
    void onBtnCreateNetClicked();

signals:
    void disConnSparedNet(QString type);

    void waitWifiStop();
    void waitLanStop();
    void onLineEditClicked();
    void onConnectChanged(); // 网络连接变化时发送该信号
};

#endif 
