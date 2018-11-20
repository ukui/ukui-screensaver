#include "authdialog.h"
#include "ui_authdialog.h"

#include <QDebug>
#include <QPixmap>
#include <QHBoxLayout>

#include <unistd.h>
#include <pwd.h>

#include "users.h"
#include "util.h"
#include "bioauth.h"
#include "biodevices.h"
#include "biodeviceswidget.h"

AuthDialog::AuthDialog(const UserItem &user, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AuthDialog),
    user(user),
    auth(new AuthPAM(this)),
    bioAuth(nullptr),
    deviceInfo(nullptr),
    bioDevices(new BioDevices(this)),
    widgetDevices(nullptr),
    movieTimer(nullptr),
    waitTimer(nullptr),
    page(Page::UNDEFINED),
    enableBiometric(false),
    firstBioAuth(true),
    authFailed(false)
{
    ui->setupUi(this);

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
        auth->authenticate(user.name);
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
    /* set user's name and avatar */
    QPixmap avatar(user.icon);
    avatar = avatar.scaled(128, 128);//ui->lblAvatar->width(), ui->lblAvatar->height());
    ui->lblAvatar->setPixmap(avatar);
    ui->lblUserName->setText(user.realName);

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
    connect(ui->lineEditPasswd, &QLineEdit::returnPressed,
            this, &AuthDialog::onRespond);
    connect(ui->btnUnlock, &QPushButton::clicked,
            this, &AuthDialog::onRespond);
    connect(ui->btnEchoMode, &QPushButton::clicked, this, [&]{
        setEchoMode(ui->lineEditPasswd->echoMode() != QLineEdit::Normal);
    });

    ui->lblCapsLock->setVisible(checkCapsLockState());


    //对话框切换按钮
    connect(ui->btnMoreDevices, &QPushButton::clicked,
            this, &AuthDialog::switchToDevices);
    connect(ui->btnToPassword, &QPushButton::clicked,
            this, &AuthDialog::switchToPassword);
    connect(ui->btnToBiometric, &QPushButton::clicked,
            this, &AuthDialog::switchToBiometric);
    connect(ui->btnRetry, &QPushButton::clicked,
            this, &AuthDialog::onBioAuthStart);

    ui->widgetSwitch->hide();
    setFocusProxy(ui->lineEditPasswd);
}

void AuthDialog::resizeEvent(QResizeEvent *event)
{
    ui->widgetBiometric->move(ui->widgetBiometric->x(),
                              ui->widgetUser->geometry().bottom());
    ui->widgetPassword->move(ui->widgetPassword->x(),
                             ui->widgetUser->geometry().bottom());
    ui->widgetMain->move(0, (height() - (ui->widgetUser->height()
                                         + ui->widgetBiometric->height())) / 2);
    ui->widgetSwitch->move(0, height() - 60);
}

void AuthDialog::closeEvent(QCloseEvent *event)
{
    qDebug() << "AuthDialog::closeEvent";
    if(bioAuth && bioAuth->isAuthenticating())
    {
        bioAuth->stopAuth();
    }

    if(auth && auth->isAuthenticating())
    {
        auth->stopAuth();
    }
    return QWidget::closeEvent(event);
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
    stopWaiting();

    if(prompt == BIOMETRIC_PAM)
    {
        page = BIOMETRIC;
        switchToBiometric();
    }
    else
    {
        authFailed = false;
        page = PASSWORD;
        ui->widgetInput->setEnabled(true);
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
        auth->authenticate(user.name);
    }
}

void AuthDialog::onRespond()
{
    ui->listWidgetMessage->clear();
    startWaiting();
    ui->widgetInput->setEnabled(false);

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
        auth->authenticate(user.name);
        return;
    }

    //使用密码认证，失败后，从新开始认证，则跳过生物识别认证
    if(authFailed)
    {
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
        deviceInfo = bioDevices->getDefaultDevice(user.uid);
        if(!deviceInfo)
        {
           switchToPassword();
           return;
        }
    }
    else if(!deviceInfo)
    {
        if(widgetDevices)
        {
            deviceInfo = widgetDevices->getSelectedDevice();
            qDebug() << "-----" << deviceInfo->device_shortname;
        }
        if(!deviceInfo)
        {
            deviceInfo = bioDevices->getFirstDevice();
        }
    }
    qDebug() << deviceInfo->device_shortname;

    page = BIOMETRIC;
    setSwitchButton();
    onBioAuthStart();
}

void AuthDialog::switchToDevices()
{
    qDebug() << "switch to select device";
    onBioAuthStop();

    deviceInfo = nullptr;

    if(!widgetDevices)
    {
        widgetDevices = new BioDevicesWidget(this);
    }
    Page pageSaved = page;
    page = DEVICES;
    setSwitchButton();
    ui->widgetUser->hide();
    ui->widgetPassword->hide();
    ui->widgetBiometric->hide();
    widgetDevices->show();
    widgetDevices->move(0, 150);
    widgetDevices->init(user.uid);

    //还原原来的page，防止切换认证模式时出错
    page = pageSaved;
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
        ui->listWidgetMessage->clear();
    }
    else if(page == DEVICES)
    {
        ui->btnToBiometric->show();
        ui->btnToPassword->show();
    }
}

void AuthDialog::onBioAuthStart()
{
    if(bioAuth)
    {
        delete bioAuth;
        bioAuth = nullptr;
    }
    if(!bioAuth)
    {
        bioAuth = new BioAuth(user.uid, *deviceInfo, this);
        connect(bioAuth, &BioAuth::notify, this, [&](const QString &notify){
            ui->lblBioNotify->setText(notify);
        });
        connect(bioAuth, &BioAuth::authComplete, this, [&](uid_t uid, bool ret){
            if(user.uid == uid && ret)
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

static int count = 0;

void AuthDialog::setBioImage(bool isGif)
{
    if(!deviceInfo)
    {
        return;
    }

    if(isGif)
    {
        if(!movieTimer)
        {
            movieTimer = new QTimer(this);
            movieTimer->setInterval(100);
            connect(movieTimer, &QTimer::timeout, this, &AuthDialog::setBioMovieImage);
        }
        count = 0;
        movieTimer->start();
    }
    else
    {
        QString type = bioTypeToString(deviceInfo->biotype);
        QString imagePath = QString("/usr/share/ukui-biometric/images/%1.png").arg(type);
        QPixmap image(imagePath);
        image = image.scaled(QSize(ui->lblBioImage->width(), ui->lblBioImage->height()));
        ui->lblBioImage->setPixmap(image);
        movieTimer->stop();
    }

    QString deviceName = deviceInfo->device_shortname;
    ui->lblBioDeviceName->setText("Current Device: " + deviceName);
}

void AuthDialog::setBioMovieImage()
{
    count++;
    QString type = bioTypeToString(deviceInfo->biotype);
    QString fileName = (count < 10 ? "0" : "") + QString::number(count);
    QString imagePath = QString("/usr/share/ukui-biometric/images/%1/%2.png")
            .arg(type).arg(fileName);
    QPixmap image(imagePath);
    image = image.scaled(QSize(ui->lblBioImage->width(), ui->lblBioImage->height()));
    ui->lblBioImage->setPixmap(image);

    if(count == 18)
    {
        count = 0;
    }
}


void AuthDialog::startWaiting()
{
    if(!waitTimer)
    {
        waitTimer = new QTimer(this);
        connect(waitTimer, &QTimer::timeout, this, &AuthDialog::updateIcon);
    }
    waitTimer->setInterval(100);
    waitImage.load(":/image/assets/waiting.png");
    waitTimer->start();
}

void AuthDialog::stopWaiting()
{
    if(waitTimer && waitTimer->isActive())
    {
        waitTimer->stop();
        ui->btnUnlock->setIcon(QIcon());
    }
}

void AuthDialog::updateIcon()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    waitImage = waitImage.transformed(matrix, Qt::FastTransformation);
    ui->btnUnlock->setIcon(QIcon(waitImage));
}
