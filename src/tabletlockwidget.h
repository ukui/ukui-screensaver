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
 * Authors: ZHAI Kang-ning <zhaikangning@kylinos.cn>
**/
#ifndef TABLETLOCKWIDGET_H
#define TABLETLOCKWIDGET_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QWidget>
#include <QResizeEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class VirtualKeyboard;
class PowerManager;
class AuthDialog;
class Users;
class UserItem;
class DisplayManager;
class QMenu;
class DigitalAuthDialog;
class SwitchButtonGroup;
class WeChatAuthDialog;
class VerificationWidget;
class VerticalVerificationWidget;
class EduPlatformInterface;
class KylinNM;

class TabletLockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabletLockWidget(QWidget *parent = nullptr);
    ~TabletLockWidget();

    enum pageType{
        pinLogin = 0,
        wecharLogin,
        check
    };

    void closeEvent(QCloseEvent *event);
    void startAuth();
    void stopAuth();
//    void RecieveKey(int key);

Q_SIGNALS:
    void closed();
    void capsLockChanged();
    void screenSaver();
    void blackSaver();

private:
    void initUI();
    void initConnect();

    void onSwitchPage();
    QWidget* getCurrentWidget();
    EduPlatformInterface* getEduPlatformInterface();
    void updateNetIcon(int status);

    void paintEvent(QPaintEvent *event) override;

    bool isOrientation();
    void showVerificationPage();

private Q_SLOTS:
    void showPowerManager();
    void showNetManager();
    void showVirtualKeyboard(bool isShow = false);
    void setVirkeyboardPos();
    void switchLoginType(int position);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    AuthDialog          *authDialog;
    VirtualKeyboard     *m_vKeyboard;
    PowerManager        *powermanager;
    QTimer              *timer;
    QMenu               *usersMenu;
    Users               *users;
    DisplayManager      *displayManager;
    DigitalAuthDialog   *m_digitalAuthDialog;
    WeChatAuthDialog    *m_weChatAuthDialog;
    SwitchButtonGroup   *m_switchButtonGroup;
    QPushButton         *m_pb_powerManager;
    QPushButton         *m_pb_networkManager;
    QPushButton         *m_cancelOrBack;
    QPushButton         *m_pb_skip;
    QPushButton         *m_vKeyboardPB;
    VerificationWidget     *m_verificationWidget;
    VerticalVerificationWidget  *m_verticalVerificationWidget;
    EduPlatformInterface  *m_eduPlatformInterface;

    KylinNM             *m_kylinNM;
    QWidget             *m_NetManagerWidget;

    int m_authType;
    int m_pageType = 0;
};

#endif // TABLETLOCKWIDGET_H
