#include "authdialog.h"
#include "ui_authdialog.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QMovie>
#include <QPixmap>
#include <QHBoxLayout>

#include <unistd.h>
#include <pwd.h>

#include "util.h"
#include "bioauth.h"
#include "biodevices.h"
#include "biodeviceswidget.h"

AuthDialog::AuthDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AuthDialog),
    auth(new AuthPAM(this)),
    bioAuth(nullptr),
    deviceInfo(nullptr),
    bioDevices(new BioDevices(this)),
    widgetDevices(nullptr),
    movie(nullptr),
    page(Page::UNDEFINED),
    enableBiometric(false),
    firstBioAuth(true),
    authFailed(false)
{
    ui->setupUi(this);

    //要认证的用户
    userId = getuid();
    struct passwd *pwd = getpwuid(userId);
    userName = QString(pwd->pw_name);

    initUI();

    connect(auth, &Auth::showMessage, this, &AuthDialog::onShowMessage);
    connect(auth, &Auth::showPrompt, this, &AuthDialog::onShowPrompt);
    connect(auth, &Auth::authenticateComplete, this, &AuthDialog::onAuthComplete);

}

AuthDialog::~AuthDialog()
{
    delete ui;
}

void AuthDialog::startAuth()
{
    qDebug() << page;
    if(!auth->isAuthenticating())
    {
        auth->authenticate(userName);
    }
    else if(page == BIOMETRIC)
    {
        onBioAuthStart();
    }
}

void AuthDialog::stopAuth()
{
    if(page == BIOMETRIC)
    {
        onBioAuthStop();
    }
}

void AuthDialog::initUI()
{
    QPixmap avatar(getUserAvatarPath(userName));

    /* set user's name and avatar */
    avatar = avatar.scaled(128, 128);//ui->lblAvatar->width(), ui->lblAvatar->height());
    ui->lblAvatar->setPixmap(avatar);
    ui->lblUserName->setText(userName);

    /* Put the button in the LineEdit */
    QHBoxLayout *hLayoutPwd = new QHBoxLayout;
    hLayoutPwd->setSpacing(0);
    hLayoutPwd->setContentsMargins(1, 1, 1, 1);
    hLayoutPwd->addStretch();
    hLayoutPwd->addWidget(ui->lblCapsLock);
    hLayoutPwd->addWidget(ui->btnEchoMode);
    hLayoutPwd->addWidget(ui->btnUnlock);
    ui->lblCapsLock->setPixmap(QPixmap(":/image/assets/warn.png"));
    ui->lineEditPasswd->setLayout(hLayoutPwd);
    ui->lineEditPasswd->setTextMargins(1, 1, ui->btnUnlock->width() +
                                         ui->btnEchoMode->width(), 1);
    ui->lineEditPasswd->setFocus();
    ui->widgetInput->hide();
    connect(ui->lineEditPasswd, &QLineEdit::returnPressed, this, &AuthDialog::onRespond);
    connect(ui->btnUnlock, &QPushButton::clicked, this, &AuthDialog::onRespond);

    ui->lblCapsLock->setVisible(checkCapsLockState());


    //对话框切换按钮
    connect(ui->btnMoreDevices, &QPushButton::clicked, this, &AuthDialog::switchToDevices);
    connect(ui->btnToPassword, &QPushButton::clicked, this, &AuthDialog::switchToPassword);
    connect(ui->btnToBiometric, &QPushButton::clicked, this, &AuthDialog::switchToBiometric);
    connect(ui->btnRetry, &QPushButton::clicked, this, &AuthDialog::onBioAuthStart);

    ui->widgetSwitch->hide();
    setFocusProxy(ui->lineEditPasswd);
}

void AuthDialog::resizeEvent(QResizeEvent *event)
{
    ui->widgetBiometric->move(ui->widgetBiometric->x(),
                              ui->widgetUser->geometry().bottom());
    ui->widgetPassword->move(ui->widgetPassword->x(),
                             ui->widgetUser->geometry().bottom());
    ui->widgetMain->move(0, (height() - ui->widgetMain->height()) / 2);
    ui->widgetSwitch->move(0, height() - 60);
}

void AuthDialog::setEchoMode(bool visible)
{
    if(visible) {
        ui->lineEditPasswd->setEchoMode(QLineEdit::Normal);
        ui->btnEchoMode->setIcon(QIcon(":/image/assets/show-password.png"));
    } else {
        ui->lineEditPasswd->setEchoMode(QLineEdit::Password);
        ui->btnEchoMode->setIcon(QIcon(":/image/assets/hide-password.png"));
    }
}

QString AuthDialog::getUserAvatarPath(const QString &username)
{
    QString iconPath;
    QDBusInterface userIface( "org.freedesktop.Accounts",
                    "/org/freedesktop/Accounts",
                    "org.freedesktop.Accounts",
                    QDBusConnection::systemBus());
    if (!userIface.isValid())
        qDebug() << "userIface is invalid";
    QDBusReply<QDBusObjectPath> userReply = userIface.call("FindUserByName",
                                username);
    if (!userReply.isValid()) {
        qDebug() << "userReply is invalid";
        iconPath = "/usr/share/kylin-greeter/default_face.png";
    }
    QDBusInterface iconIface( "org.freedesktop.Accounts",
                    userReply.value().path(),
                    "org.freedesktop.DBus.Properties",
                    QDBusConnection::systemBus());
    if (!iconIface.isValid())
        qDebug() << "IconIface is invalid";
    QDBusReply<QDBusVariant> iconReply = iconIface.call("Get",
                "org.freedesktop.Accounts.User", "IconFile");
    if (!iconReply.isValid()) {
        qDebug() << "iconReply is invalid";
        iconPath = "/usr/share/kylin-greeter/default_face.png";
    }
    iconPath = iconReply.value().variant().toString();
    if (access(iconPath.toLocal8Bit().data(), R_OK) != 0) /* No Access Permission */
        qDebug() << "Can't access user avatar:" << iconPath
                        << "No access permission.";
    return iconPath;
}

void AuthDialog::onShowMessage(const QString &message, Auth::MessageType type)
{
    QListWidgetItem *item = new QListWidgetItem(ui->listWidgetMessage);
    item->setText(message);
    item->setTextAlignment(Qt::AlignCenter);
    if(type == Auth::MessageTypeError)
        item->setForeground(Qt::yellow);

    ui->listWidgetMessage->addItem(item);
}

void AuthDialog::onShowPrompt(const QString &prompt, Auth::PromptType type)
{
    if(prompt == BIOMETRIC_PAM)
    {
        page = BIOMETRIC;
        switchToBiometric();
    }
    else
    {
        authFailed = false;
        page = PASSWORD;
        switchToPassword();

        ui->widgetInput->show();
        ui->lineEditPasswd->setPlaceholderText(prompt);
        ui->lineEditPasswd->setFocus();

        setEchoMode(type == Auth::PromptTypeQuestion);
    }
}

void AuthDialog::onAuthComplete()
{
    if(auth->isAuthenticated())
    {
        Q_EMIT authenticateCompete(true);
    }
    else
    {
        onShowMessage(tr("Password Incorrect, Please try again"),
                      Auth::MessageTypeError);
        //认证失败，重新认证
        authFailed = true;
        ui->lineEditPasswd->clear();
        auth->authenticate(userName);
    }
}

void AuthDialog::onRespond()
{
    ui->listWidgetMessage->clear();
    auth->respond(ui->lineEditPasswd->text());
}

void AuthDialog::onCapsLockChanged()
{
    ui->lblCapsLock->setVisible(!ui->lblCapsLock->isVisible());
}

void AuthDialog::switchToPassword()
{
    qDebug() << "switch to password auth";
    ui->widgetUser->show();
    ui->widgetBiometric->hide();
    ui->widgetPassword->show();
    if(widgetDevices)
    {
        widgetDevices->hide();
    }

    if(bioAuth && bioAuth->isAuthenticating())
        bioAuth->stopAuth();

    if(page == BIOMETRIC)
        auth->respond(BIOMETRIC_IGNORE);

    page = PASSWORD;
    setSwitchButton();
}

void AuthDialog::switchToBiometric()
{
    //是从密码认证切换到生物识别认证的，需要从头开始走PAM流程
    if(page == PASSWORD)
    {
        auth->authenticate(userName);
        return;
    }

    //使用密码认证，失败后，从新开始认证，则跳过生物识别认证
    if(authFailed)
    {
        auth->respond(BIOMETRIC_IGNORE);
        switchToPassword();
        return;
    }

    //没有可用的生物识别设备，跳过生物识别认证，进入下一个PAM模块
    if(bioDevices->count() <= 0)
    {
        enableBiometric = false;
        qDebug() << "hasn't available device";
        auth->respond(BIOMETRIC_IGNORE);
        switchToPassword();
        return;
    }
    enableBiometric = true;

    qDebug() << "switch to biometric auth";
    ui->widgetUser->show();
    ui->widgetPassword->hide();
    ui->widgetBiometric->show();
    if(widgetDevices)
    {
        widgetDevices->hide();
    }

    //第一次接收到生物识别认证的PAM消息时，如果没有配置默认设备，则直接跳过生物识别认证
    if(firstBioAuth)
    {
        firstBioAuth = false;
        deviceInfo = bioDevices->getDefaultDevice(userId);
        if(!deviceInfo)
        {
           switchToPassword();
           return;
        }
    }
    else if(!deviceInfo)
    {
        deviceInfo = bioDevices->getFirstDevice();
    }
    qDebug() << deviceInfo->device_shortname;

    page = BIOMETRIC;
    setSwitchButton();
    onBioAuthStart();
}

void AuthDialog::switchToDevices()
{
    onBioAuthStop();

    if(!widgetDevices)
    {
        widgetDevices = new BioDevicesWidget(this);
        connect(widgetDevices, &BioDevicesWidget::deviceChanged,
                this, &AuthDialog::onBioAuthDeviceChanged);
        connect(widgetDevices, &BioDevicesWidget::back,
                this, [&]{
            widgetDevices->hide();
            ui->widgetUser->show();
            ui->widgetBiometric->show();
        });
    }
    page = DEVICES;
    setSwitchButton();
    ui->widgetUser->hide();
    ui->widgetPassword->hide();
    ui->widgetBiometric->hide();
    widgetDevices->show();
    widgetDevices->move(0, 100);
    widgetDevices->init(userId);
}

void AuthDialog::setSwitchButton()
{
    qDebug() << page;
    ui->widgetSwitch->show();
    ui->btnMoreDevices->hide();
    ui->btnToBiometric->hide();
    ui->btnToPassword->hide();
    ui->btnRetry->hide();

    if(page == PASSWORD)
    {
        if(enableBiometric)
        {
            ui->btnToBiometric->show();
            if(bioDevices->count() > 1)
                ui->btnMoreDevices->show();
        }
    }
    else if(page == BIOMETRIC)
    {
        if(bioAuth &&!bioAuth->isAuthenticating())
            ui->btnRetry->show();
        if(bioDevices->count() > 1)
            ui->btnMoreDevices->show();
        ui->btnToPassword->show();
    }
    else if(page == DEVICES)
    {
        ui->btnToBiometric->show();
        ui->btnToPassword->show();
    }
}

void AuthDialog::onBioAuthStart()
{
    if(!bioAuth)
    {
        bioAuth = new BioAuth(userId, *deviceInfo, this);
        connect(bioAuth, &BioAuth::notify, this, [&](const QString &notify){
            ui->lblBioNotify->setText(notify);
        });
        connect(bioAuth, &BioAuth::authComplete, this, [&](uid_t uid, bool ret){
            if(uid == userId && ret)
            {
                auth->respond(BIOMETRIC_SUCCESS);
            }
            else
            {
                onBioAuthStop();
    //            QTimer::singleShot(2000, this, SLOT(onBioAuthStart()));
            }
        });
    }
    bioAuth->startAuth();
    setBioImage(true);
    setSwitchButton();
}

void AuthDialog::onBioAuthStop()
{
    if(bioAuth && bioAuth->isAuthenticating())
    {
        bioAuth->stopAuth();
    }
    setBioImage(false);
    setSwitchButton();
}

void AuthDialog::setBioImage(bool isGif)
{
    if(!deviceInfo)
    {
        return;
    }

    QString deviceName = deviceInfo->device_shortname;
    QString type = BioDevices::bioTypeToString_tr(deviceInfo->biotype);
    QString imagePath;
    if(isGif)
    {
        if(movie)
        {
            movie->deleteLater();
        }

        imagePath = QString("/usr/share/ukui-biometric/images/%1.gif").arg(type);
        movie = new QMovie(imagePath, "", this);
        movie->setScaledSize(QSize(ui->lblBioImage->width(), ui->lblBioImage->height()));
        movie->start();
        ui->lblBioImage->setMovie(movie);
    }
    else
    {
        imagePath = QString("/usr/share/ukui-biometric/images/%1.png").arg(type);
        QPixmap image(imagePath);
        image = image.scaled(QSize(ui->lblBioImage->width(), ui->lblBioImage->height()));
        ui->lblBioImage->setPixmap(image);
    }
    ui->lblBioDeviceName->setText("Current Device: " + deviceName);
}

void AuthDialog::onBioAuthDeviceChanged(DeviceInfo *device)
{
    deviceInfo = device;
    qDebug() << deviceInfo->device_shortname;
    bioAuth->deleteLater();
    bioAuth = nullptr;
    onBioAuthStart();
}
