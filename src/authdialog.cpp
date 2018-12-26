#include "authdialog.h"

#include <QDebug>
#include <QPixmap>
#include <QHBoxLayout>

#include <unistd.h>
#include <pwd.h>

#include "users.h"
#include "iconedit.h"
#include "biometricproxy.h"
#include "biometricauthwidget.h"
#include "biometricdeviceswidget.h"

AuthDialog::AuthDialog(const UserItem &user, QWidget *parent) :
    QWidget(parent),
    user(user),
    auth(new AuthPAM(this)),
    m_deviceCount(-1),
    authMode(UNKNOWN),
    m_biometricProxy(nullptr),
    m_biometricAuthWidget(nullptr),
    m_biometricDevicesWidget(nullptr),
    m_buttonsWidget(nullptr)
{
    initUI();

    connect(auth, &Auth::showMessage, this, &AuthDialog::onShowMessage);
    connect(auth, &Auth::showPrompt, this, &AuthDialog::onShowPrompt);
    connect(auth, &Auth::authenticateComplete, this, &AuthDialog::onAuthComplete);

}

void AuthDialog::startAuth()
{
    auth->authenticate(user.name);

    showPasswordAuthWidget();
    m_passwordEdit->clear();
    m_passwordEdit->setEnabled(false);
}

void AuthDialog::stopAuth()
{
    //这里只是为了在显示屏幕保护程序时停止生物识别认证

    if(m_biometricAuthWidget)
        m_biometricAuthWidget->stopAuth();
}

void AuthDialog::initUI()
{
    setFixedWidth(500);

    m_userWidget = new QWidget(this);
    m_userWidget->setObjectName(QStringLiteral("userWidget"));

    /* 头像 */
    m_faceLabel = new QLabel(m_userWidget);
    m_faceLabel->setObjectName(QStringLiteral("faceLabel"));
    m_faceLabel->setFocusPolicy(Qt::NoFocus);
    QPixmap facePixmap(user.icon);
    facePixmap = facePixmap.scaled(128, 128, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    m_faceLabel->setPixmap(facePixmap);

    /* 用户名 */
    m_nameLabel = new QLabel(m_userWidget);
    m_nameLabel->setObjectName(QStringLiteral("login_nameLabel"));
    m_nameLabel->setFocusPolicy(Qt::NoFocus);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setText(user.realName.isEmpty() ? user.name : user.realName);

    /* 密码框所在窗口 */
    m_passwdWidget = new QWidget(this);
    m_passwdWidget->setObjectName(QStringLiteral("passwordWidget"));

    /* 密码框 */
    m_passwordEdit = new IconEdit(m_passwdWidget);
    m_passwordEdit->setObjectName(QStringLiteral("passwordEdit"));
    m_passwordEdit->setIcon(QIcon(":/image/assets/unlock-button.png"));
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);
    m_passwordEdit->installEventFilter(this);
//    m_passwordEdit->hide(); //收到请求密码的prompt才显示出来
    m_passwordEdit->setEnabled(false);
    connect(m_passwordEdit, SIGNAL(clicked(const QString&)),
            this, SLOT(onRespond(const QString&)));

    /* 密码认证信息显示列表 */
    m_messageLabel = new QLabel(m_passwdWidget);
    m_messageLabel->setObjectName(QStringLiteral("messageLabel"));
    m_messageLabel->setAlignment(Qt::AlignCenter);
}

void AuthDialog::resizeEvent(QResizeEvent *)
{
    setChildrenGeometry();
}

void AuthDialog::setChildrenGeometry()
{
    // 用户信息显示位置
    m_userWidget->setGeometry(0, (height() - 240 - 150) / 2,
                              width(), 240);
    m_faceLabel->setGeometry((width() - 128) / 2, 0, 128, 128);
    m_nameLabel->setGeometry(0, m_faceLabel->geometry().bottom() + 25,
                             width(), 40);

    // 密码框和提示信息显示位置
    m_passwdWidget->setGeometry(0, m_userWidget->geometry().bottom(), width(), 150);
    m_passwordEdit->setGeometry((m_passwdWidget->width() - 400)/2, 0, 400, 40);
    m_messageLabel->setGeometry((m_passwdWidget->width() - 300)/2,
                                m_passwordEdit->geometry().bottom() + 25,
                                300, 20);


    setBiometricWidgetGeometry();
    setBiometricButtonWidgetGeometry();
}



void AuthDialog::closeEvent(QCloseEvent *event)
{
    qDebug() << "AuthDialog::closeEvent";

    if(auth && auth->isAuthenticating())
    {
        auth->stopAuth();
    }
    return QWidget::closeEvent(event);
}


void AuthDialog::onShowMessage(const QString &message, Auth::MessageType type)
{
    m_messageLabel->setText(message);
    stopWaiting();
}

void AuthDialog::onShowPrompt(const QString &prompt, Auth::PromptType type)
{
    qDebug() << "prompt: " << prompt;
    QString text = prompt;
    if(text == BIOMETRIC_PAM)
    {
        if(authMode == PASSWORD)
        {
            skipBiometricAuth();
        }
        else
        {
            performBiometricAuth();
        }
    }
    else
    {
        stopWaiting();
        if(!text.isEmpty())
            m_passwordEdit->setEnabled(true);

        m_passwordEdit->setFocus();
        if(type != Auth::PromptTypeSecret)
            m_passwordEdit->setType(QLineEdit::Normal);
        else
            m_passwordEdit->setType(QLineEdit::Password);

        if(text == "Password: ")
            text = tr("Password: ");

        m_passwordEdit->clear();
        m_passwordEdit->setPrompt(text);
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
        authMode = PASSWORD;
        startAuth();
    }
}

void AuthDialog::onRespond(const QString &text)
{
    clearMessage();
    startWaiting();

    auth->respond(text);
}

void AuthDialog::onCapsLockChanged()
{
    m_passwordEdit->onCapsStateChanged();
}

void AuthDialog::startWaiting()
{
    if(m_buttonsWidget)
    {
        m_buttonsWidget->setEnabled(false);
    }
}

void AuthDialog::stopWaiting()
{
    m_passwordEdit->stopWaiting();
    if(m_buttonsWidget)
    {
        m_buttonsWidget->setEnabled(true);
    }
}

void AuthDialog::clearMessage()
{
    m_messageLabel->clear();
}

void AuthDialog::performBiometricAuth()
{
    if(!m_biometricProxy)
    {
        m_biometricProxy = new BiometricProxy(this);
    }

    //服务没启动，或者打开DBus连接出错
    if(!m_biometricProxy->isValid())
    {
        qWarning() << "An error occurs when connect to the biometric DBus";
        skipBiometricAuth();
        return;
    }

    //初始化enableBiometriAuth
    if(m_deviceCount < 0)
    {
        m_deviceCount = m_biometricProxy->GetDevCount();
    }

    //没有可用设备，不启用生物识别认证
    if(m_deviceCount < 1)
    {
        qWarning() << "No available devices";
        skipBiometricAuth();
        return;
    }

    initBiometricButtonWidget();

    //获取默认设备
    if(m_deviceName.isEmpty())
    {
        m_deviceName = GetDefaultDevice(user.name);
    }
    qDebug() << m_deviceName;
//    qDebug() << (m_deviceInfo ? m_deviceInfo->shortName : "");

    //如果默认设备为空的话，第一次不启动生物识别认证
    if(m_deviceName.isEmpty() && !m_deviceInfo)
    {
        qDebug() << "No default device";
        skipBiometricAuth();
        return;
    }

    //初始化生物识别认证UI
    initBiometricWidget();
    clearMessage();

    if(!m_deviceInfo)
    {
        m_deviceInfo = m_biometricDevicesWidget->findDeviceByName(m_deviceName);
    }
    if(!m_deviceInfo)
    {
        skipBiometricAuth();
        return;
    }

    authMode = BIOMETRIC;

    m_biometricAuthWidget->startAuth(m_deviceInfo, user.uid);

    showBiometricAuthWidget();
}

void AuthDialog::skipBiometricAuth()
{
    auth->respond(BIOMETRIC_IGNORE);

    showPasswordAuthWidget();
}

void AuthDialog::initBiometricWidget()
{
    if(m_biometricAuthWidget)
    {
        return;
    }

    m_biometricAuthWidget = new BiometricAuthWidget(m_biometricProxy, this);
    connect(m_biometricAuthWidget, &BiometricAuthWidget::authComplete,
            this, &AuthDialog::onBiometricAuthComplete);
    m_biometricDevicesWidget = new BiometricDevicesWidget(m_biometricProxy, this);
    connect(m_biometricDevicesWidget, &BiometricDevicesWidget::deviceChanged,
            this, &AuthDialog::onDeviceChanged);

    setBiometricWidgetGeometry();
}

void AuthDialog::initBiometricButtonWidget()
{
    if(m_buttonsWidget)
    {
        return;
    }

    m_buttonsWidget = new QWidget(this);
    m_buttonsWidget->setObjectName(QStringLiteral("buttonsWidget"));
    m_buttonsWidget->setFixedHeight(25);

    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    m_biometricButton = new QPushButton(m_buttonsWidget);
    m_biometricButton->setObjectName(QStringLiteral("biometricButton"));
    m_biometricButton->setText(tr("Biometric Authentication"));
    m_biometricButton->setSizePolicy(sizePolicy);
    m_biometricButton->setVisible(false);
    m_biometricButton->setCursor(Qt::PointingHandCursor);
    QFontMetrics fm(m_biometricButton->font(), m_biometricButton);
    int width = fm.width(m_biometricButton->text());
    m_biometricButton->setMaximumWidth(width + 20);
    connect(m_biometricButton, &QPushButton::clicked,
            this, &AuthDialog::onBiometricButtonClicked);

    m_passwordButton = new QPushButton(m_buttonsWidget);
    m_passwordButton->setObjectName(QStringLiteral("passwordButton"));
    m_passwordButton->setText(tr("Password Authentication"));
    fm = QFontMetrics(m_passwordButton->font(), m_passwordButton);
    width = fm.width(m_passwordButton->text());
    m_passwordButton->setMaximumWidth(std::max(width + 20, 110));
    m_passwordButton->setSizePolicy(sizePolicy);
    m_passwordButton->setVisible(false);
    m_passwordButton->setCursor(Qt::PointingHandCursor);
    connect(m_passwordButton, &QPushButton::clicked,
            this, &AuthDialog::onPasswordButtonClicked);

    m_otherDeviceButton = new QPushButton(m_buttonsWidget);
    m_otherDeviceButton->setObjectName(QStringLiteral("otherDeviceButton"));
    m_otherDeviceButton->setText(tr("Other Devices"));
    m_otherDeviceButton->setSizePolicy(sizePolicy);
    m_otherDeviceButton->setMaximumWidth(std::max(width + 20, 110));
    m_otherDeviceButton->setVisible(false);
    m_otherDeviceButton->setCursor(Qt::PointingHandCursor);
    connect(m_otherDeviceButton, &QPushButton::clicked,
            this, &AuthDialog::onOtherDevicesButtonClicked);

    m_retryButton = new QPushButton(m_buttonsWidget);
    m_retryButton->setObjectName(QStringLiteral("retryButton"));
    m_retryButton->setText(tr("Retry"));
    m_retryButton->setSizePolicy(sizePolicy);
    m_retryButton->setMaximumWidth(110);
    m_retryButton->setVisible(false);
    m_retryButton->setCursor(Qt::PointingHandCursor);
    connect(m_retryButton, &QPushButton::clicked,
            this, &AuthDialog::onRetryButtonClicked);

    QGridLayout *layout = new QGridLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_biometricButton, 0, 0);
    layout->addWidget(m_passwordButton, 0, 1);
    layout->addWidget(m_otherDeviceButton, 0, 2);
    layout->addWidget(m_retryButton, 0, 3);

    m_buttonsWidget->setLayout(layout);
    m_buttonsWidget->show();

    setBiometricButtonWidgetGeometry();
}

void AuthDialog::setBiometricWidgetGeometry()
{
    //生物识别
    if(m_biometricAuthWidget)
    {
        m_biometricAuthWidget->setGeometry(0, m_userWidget->geometry().bottom(),
                                           width(), m_biometricAuthWidget->height());
    }
    if(m_biometricDevicesWidget)
    {
        m_biometricDevicesWidget->setGeometry((width() - m_biometricDevicesWidget->width()) / 2,
                                              (height() - m_biometricDevicesWidget->height()) / 2,
                                              m_biometricDevicesWidget->width(),
                                              m_biometricDevicesWidget->height());
    }
}

void AuthDialog::setBiometricButtonWidgetGeometry()
{
    if(m_buttonsWidget)
    {
        m_buttonsWidget->setGeometry(0, height() - m_buttonsWidget->height() - 20,
                                     width(), m_buttonsWidget->height());
    }
}

void AuthDialog::onDeviceChanged(const DeviceInfoPtr &deviceInfo)
{
    qDebug() << "device changed: " << *deviceInfo;
    m_deviceInfo = deviceInfo;

    if(authMode != BIOMETRIC)
    {
        authMode = BIOMETRIC;
        startAuth();
    }
    else
    {
        m_biometricAuthWidget->startAuth(m_deviceInfo, user.uid);
    }

    m_userWidget->show();
    m_passwdWidget->hide();
    m_biometricAuthWidget->show();
    m_buttonsWidget->show();
    m_biometricDevicesWidget->hide();
    m_biometricButton->hide();
    m_passwordButton->show();
    m_otherDeviceButton->show();
    m_retryButton->hide();
}

void AuthDialog::onBiometricAuthComplete(bool result)
{
    if(!result)
    {
        m_retryButton->setVisible(!m_biometricAuthWidget->isHidden());
    }
    else
    {
        auth->respond(BIOMETRIC_SUCCESS);
    }
}

void AuthDialog::onBiometricButtonClicked()
{
    //当前没有设备，让用户选择设备
    if(!m_deviceInfo)
    {
        m_otherDeviceButton->click();
    }
    else
    {
        authMode = BIOMETRIC;
        startAuth();
    }

}

void AuthDialog::onPasswordButtonClicked()
{
    skipBiometricAuth();
}

void AuthDialog::onOtherDevicesButtonClicked()
{
    m_biometricAuthWidget->stopAuth();

    showBiometricDeviceWidget();
}

void AuthDialog::onRetryButtonClicked()
{
    m_biometricAuthWidget->startAuth(m_deviceInfo, user.uid);
    m_retryButton->setVisible(false);
}

void AuthDialog::showPasswordAuthWidget()
{
    m_userWidget->setVisible(true);
    m_passwdWidget->setVisible(true);

    if(m_biometricAuthWidget)
    {
        m_biometricAuthWidget->setVisible(false);
        m_biometricDevicesWidget->setVisible(false);

        m_biometricAuthWidget->stopAuth();
    }

    if(m_buttonsWidget)
    {
        if(m_deviceCount > 0)
        {
            m_buttonsWidget->setVisible(true);
            m_biometricButton->setVisible(true);
            m_passwordButton->setVisible(false);
            m_otherDeviceButton->setVisible(false);
            m_retryButton->setVisible(false);
        }
        else
        {
            qDebug() << "hide buttons widget";
            m_buttonsWidget->setVisible(false);
        }
    }
}

void AuthDialog::showBiometricAuthWidget()
{
    m_userWidget->setVisible(true);
    m_passwdWidget->setVisible(false);

    if(m_biometricAuthWidget)
    {
        m_biometricAuthWidget->setVisible(true);
        m_biometricDevicesWidget->setVisible(false);
    }

    if(m_buttonsWidget && m_deviceCount > 0)
    {
        m_buttonsWidget->setVisible(true);
        m_biometricButton->setVisible(false);
        m_passwordButton->setVisible(true);
        m_otherDeviceButton->setVisible(m_deviceCount > 1);
        m_retryButton->setVisible(!m_biometricAuthWidget->isAuthenticating());
    }
    else
    {
        m_buttonsWidget->setVisible(false);
    }
}

void AuthDialog::showBiometricDeviceWidget()
{
    m_userWidget->setVisible(false);
    m_passwdWidget->setVisible(false);

    if(m_biometricAuthWidget)
    {
        m_biometricAuthWidget->setVisible(false);
        m_biometricDevicesWidget->setVisible(true);
    }

    if(m_buttonsWidget)
    {
        m_buttonsWidget->setVisible(false);
        m_biometricButton->setVisible(false);
        m_passwordButton->setVisible(false);
        m_otherDeviceButton->setVisible(false);
        m_retryButton->setVisible(false);
    }
}
