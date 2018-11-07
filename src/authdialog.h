#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QWidget>
#include "auth-pam.h"
#include "types.h"

namespace Ui {
class AuthDialog;
}

class Auth;
class BioAuth;
class BioDevices;
class DeviceInfo;
class BioDevicesWidget;
class AuthDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AuthDialog(QWidget *parent = 0);
    ~AuthDialog();
    void resizeEvent(QResizeEvent *event);

private:
    void initUI();
    void setEchoMode(bool visible);
    QString getUserAvatarPath(const QString &username);
    void setBioImage(bool isGif);
    void setSwitchButton();

private Q_SLOTS:
    void onShowMessage(const QString &message, Auth::MessageType type);
    void onShowPrompt(const QString &prompt, Auth::PromptType type);
    void onAuthComplete();
    void onRespond();
    void onBioAuthStart();
    void onBioAuthStop();

public Q_SLOTS:
    void switchToBiometric();
    void switchToPassword();
    void switchToDevices();
    void onCapsLockChanged(bool state);

Q_SIGNALS:
    void authenticateCompete(bool result);

private:
    enum Page
    {
        BIOMETRIC,
        PASSWORD,
        DEVICES
    };

    Ui::AuthDialog      *ui;
    Auth                *auth;
    BioAuth             *bioAuth;
    DeviceInfo          *deviceInfo;
    BioDevices          *bioDevices;
    BioDevicesWidget    *widgetDevices;
    QMovie              *movie;
    QString             userName;
    uid_t               userId;
    Page                page;
    bool                enableBiometric;
};

#endif // AUTHDIALOG_H
