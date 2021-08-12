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
#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QWidget>
#include <QTimer>
#include "auth-pam.h"
#include "types.h"
#include "users.h"
#include "biometricdeviceinfo.h"
#include "pam-tally.h"


namespace Ui {
class AuthDialog;
}

class QLabel;
class QPushButton;
class IconEdit;
class Auth;
class BiometricProxy;
class BiometricAuthWidget;
class BiometricDevicesWidget;
class PamTally;

extern float scale;
class AuthDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AuthDialog(const UserItem &user, QWidget *parent = 0);
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);
    void setUserOfAuth();
    void setX11Focus();
private:
    void initUI();
    void startWaiting();
    void stopWaiting();
    void clearMessage();
    void performBiometricAuth();
    void skipBiometricAuth();
    void initBiometricWidget();
    void initBiometricButtonWidget();
    void setChildrenGeometry();
    void setBiometricWidgetGeometry();
    void setBiometricButtonWidgetGeometry();
    void showPasswordAuthWidget();
    void showBiometricAuthWidget();
    void showBiometricDeviceWidget();
    QPixmap PixmapToRound(const QPixmap &src, int radius);
    void startBioAuth();


private Q_SLOTS:
    void onShowMessage(const QString &message, Auth::MessageType type);
    void onShowPrompt(const QString &prompt, Auth::PromptType type);
    void onAuthComplete();
    void onRespond(const QString &text);
//    void onBioAuthStart();
//    void onBioAuthStop();
//    void setBioMovieImage();
//    void updateIcon();
    void onDeviceChanged(const DeviceInfoPtr &deviceInfo);
    void onBiometricAuthComplete(bool result);
    void onBiometricButtonClicked();
    void onPasswordButtonClicked();
    void onOtherDevicesButtonClicked();
    void onRetryButtonClicked();
    void pamBioSuccess();

public Q_SLOTS:
//    void switchToBiometric();
//    void switchToPassword();
//    void switchToDevices();
    void onCapsLockChanged();
    void startAuth();
    void stopAuth();

Q_SIGNALS:
    void authenticateCompete(bool result);

private:
    UserItem            user;
    Auth                *auth;

    enum AuthMode { PASSWORD, BIOMETRIC, UNKNOWN };

    AuthMode            authMode;

    // biometric auth
    int                     m_deviceCount;
    int                     m_featureCount;
    QString                 m_deviceName;
    DeviceInfoPtr           m_deviceInfo;
    BiometricProxy          *m_biometricProxy;
    BiometricAuthWidget     *m_biometricAuthWidget;
    BiometricDevicesWidget  *m_biometricDevicesWidget;
    QWidget                 *m_buttonsWidget;
    QPushButton             *m_biometricButton;
    QPushButton             *m_passwordButton;
    QPushButton             *m_otherDeviceButton;
    QPushButton             *m_retryButton;

    // UI
//    QPushButton     *m_backButton;         //返回用户列表
    QWidget         *m_userWidget;         //放置用户信息Label
    QLabel          *m_faceLabel;          //头像
    QLabel          *m_nameLabel;          //用户名
//    QLabel          *m_isLoginLabel;       //提示是否已登录

    QWidget         *m_passwdWidget;        //放置密码输入框和信息列表
    IconEdit        *m_passwordEdit;       //密码输入框
    QLabel          *m_messageLabel;         //PAM消息显示
    bool            usebind;
    bool            usebindstarted;
    bool            isBioPassed;
    int             maxFailedTimes;
    bool            isHiddenSwitchButton;
    int             failedTimes;
    QTimer          *m_bioTimer;
    bool            useFirstDevice;
    bool            isLockingFlg;   //判断当前是否正在锁定倒计时


    void root_unlock_countdown();
    void unlock_countdown();
    QTimer *m_timer;
};

#endif // AUTHDIALOG_H
