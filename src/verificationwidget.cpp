#include "verificationwidget.h"
#include <QGridLayout>
#include <QPainter>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRegExpValidator>
#include <QSpacerItem>
#include <QSizePolicy>

#include "common.h"

#include "wechatauthdialog.h"
#include "eduplatforminterface.h"
#include "accountsinterface.h"

/**
 * 忘记密码后，校验页面：分为二维码校验和手机号校验
 */

bool isDigitalString(QString s);//判断字符串是否全为数字

VerificationWidget::VerificationWidget(QWidget *parent) : QWidget(parent)
{
    qDebug() << "PermissionCheck";
    resize(1000,400);
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    setLayout(mainLayout);

    m_weChatWidget = new WeChatAuthDialog(1,this);
    mainLayout->addWidget(m_weChatWidget, 1, Qt::AlignLeft);
    m_weChatWidget->setFixedWidth(400);

    mainLayout->addSpacerItem(new QSpacerItem(0,200,QSizePolicy::Fixed,QSizePolicy::Expanding));

    m_phoneAuthWidget = new PhoneAuthWidget(this);
    mainLayout->addWidget(m_phoneAuthWidget, 1, Qt::AlignRight);
    m_phoneAuthWidget->setFixedWidth(400);

    connect(m_phoneAuthWidget, &PhoneAuthWidget::pageMessage, this, [=](SwitchPage s, QList<QVariant> list){
        Q_EMIT pageMessage(s, list);
    });

    connect(m_weChatWidget, &WeChatAuthDialog::qRStatusChange, this, [=] (QString loginname, QString loginpwd, int curstatus){
        //QString loginname, QString loginpwd, int curstatus
        if (loginname.isEmpty())
        {
            // Todo？正在登陆？？？？
            qDebug() << "wechar name is null";
        } else if(getenv("USER") == loginname)
        {
            Q_EMIT pageMessage(SwitchPage::SwitchToResetPWD, QList<QVariant>());;
        } else {
            qWarning() << "[error] <VerificationWidget> Username does not match wechat<" << loginname << ">" << " local<" << getenv("USER")<<">";
            m_weChatWidget->showErrorMessage(tr("Please scan by bound WeChat"));
            m_weChatWidget->onReset();
        }
    });
}

VerificationWidget::~VerificationWidget()
{
    m_phoneAuthWidget->close();
    m_weChatWidget->close();
}

void VerificationWidget::paintEvent(QPaintEvent *)
{
    int x1 = width() / 2;
    int y1 = height() / 4;
    int x2 = x1;
    int y2 = height() / 3 * 2 + y1;
    QLinearGradient linearGra(QPoint(x1, y1), QPoint(x2, y2));
    linearGra.setColorAt(0, QColor(238, 238, 238, 0));
    linearGra.setColorAt(0.5, QColor(255, 255, 255, 138));
    linearGra.setColorAt(1, QColor(216, 216, 216, 0));

    QPainter painter(this);
    QBrush brush(linearGra);
    painter.setPen(Qt::transparent);
    painter.setBrush(brush);
    painter.drawRoundedRect(QRect(x1, y1, 4, height() / 3 * 2), 16, 16);
}

void VerificationWidget::reloadQR()
{
    m_weChatWidget->reloadQR();
}

InputInfos::InputInfos(QWidget *parent):
    QWidget(parent)
{
    initUI();
    initConnect();
    setQSS();
}

void InputInfos::initUI()
{
    setFixedSize(316, 129);
    QGridLayout *mainLayout = new QGridLayout(this);
    setLayout(mainLayout);
    mainLayout->setColumnStretch(1, 0);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->setColumnStretch(3, 0);

    m_pPhoneIconLB = new QLabel(this);
    m_pPhoneIconLB->setPixmap(QPixmap(":/image/assets/intel/phone.png"));
    mainLayout->addWidget(m_pPhoneIconLB, 0, 1, 1, 1, Qt::AlignLeft);

    m_pPhoneLE = new QLineEdit(this);
    m_pPhoneLE->setReadOnly(true);
    //限制以下特殊符号在lineEdit中的输入
    /*需求变更，不需要用户输入手机号，直接获取已绑定的手机号显示给用户
    QRegExp phoneNumrx = QRegExp("^1([358][0-9]|4[579]|66|7[0135678]|9[89])[0-9]{8}$");
    QRegExpValidator *phoneNumValidator = new QRegExpValidator(phoneNumrx);
    m_pPhoneLE->setValidator(phoneNumValidator);
    */
    m_pPhoneLE->setText("正在获取手机号...");
    GetPhoneNumThread *workerThread = new GetPhoneNumThread(this);
    connect(workerThread, &GetPhoneNumThread::resultReady, this, &InputInfos::onGetPhoneNum);
    connect(workerThread, &GetPhoneNumThread::finished, workerThread, &QObject::deleteLater);
    workerThread->start();

    m_pPhoneLE->setPlaceholderText("手机号");
    m_pPhoneLE->setProperty("class", "InputLine");
    mainLayout->addWidget(m_pPhoneLE, 0, 2, 1, 2, Qt::AlignLeft);

    m_pVerCodeIconLB = new QLabel(this);
    m_pVerCodeIconLB->setPixmap(QPixmap(":/image/assets/sms.png"));
    mainLayout->addWidget(m_pVerCodeIconLB, 1, 1, 1, 1, Qt::AlignLeft);

    //限制以下特殊符号在lineEdit中的输入
    QRegExp verCoderx = QRegExp("^[0-9]{6}$");
    QRegExpValidator *verCodeValidator = new QRegExpValidator(verCoderx);
    m_pVerCodeLE = new MyLineEdit(this);
    m_pVerCodeLE->setValidator(verCodeValidator);
    mainLayout->addWidget(m_pVerCodeLE, 1, 2, 1, 1, Qt::AlignLeft);

    m_pGetVerCodeBT = new QPushButton(this);
    m_pGetVerCodeBT->setText(tr("Get code"));
    m_pGetVerCodeBT->setEnabled(false);
    m_pGetVerCodeBT->setFocusPolicy(Qt::NoFocus);
    m_pGetVerCodeBT->setProperty("class", "GetCodeBT");
    m_pGetVerCodeBT->setCursor(QCursor(Qt::PointingHandCursor));
    mainLayout->addWidget(m_pGetVerCodeBT, 1, 3, 1, 1, Qt::AlignLeft);

    m_pNextGetVerCodeQTimer = new QTimer(this);

    m_curInputState = InputState::InputWaiting;

    m_networkWatcher = new NetWorkWatcher(this);
}

void InputInfos::initConnect()
{
    connect(m_pPhoneLE, &QLineEdit::textChanged, this, &InputInfos::onPhoneTextChanged);

    connect(m_pVerCodeLE, &QLineEdit::textChanged, this, &InputInfos::onVerCodeTextChanged);

    connect(m_pNextGetVerCodeQTimer, &QTimer::timeout, this, &InputInfos::onNextGetVerCodeTimer);

    connect(m_pGetVerCodeBT, &QPushButton::clicked, this, &InputInfos::onGetVerCode);

    connect(m_networkWatcher, &NetWorkWatcher::NetworkStateChanged, this, &InputInfos::onNetworkStateChanged);
}

void InputInfos::setQSS()
{
    QString style_sheet = ".InputLine{"
                          "padding:10px 2px 10px 2px;"
                          "background-color:rgb(255,255,255,0);"
                          "color:black;"
                          "}"
                          ".GetCodeBT{"
                          "background:rgba(0,0,0,0);"
                          "color:rgba(38,38,38,115);"
                          "font-size:16px;"
                          "}";
    setStyleSheet(style_sheet);
    //adjustSize();
}

void InputInfos::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(rect(), 16, 16);
    painter.setPen(QColor(226, 226, 226));
    painter.drawLine(QLineF(0, m_pPhoneIconLB->y() + m_pPhoneIconLB->height() + 1,
                            width(), m_pPhoneIconLB->y() + m_pPhoneIconLB->height() + 1));
}

void InputInfos::onPhoneTextChanged(const QString &text)
{
    if(text.length() == 11 && isDigitalString(text)){
        m_pGetVerCodeBT->setEnabled(true);
        m_pGetVerCodeBT->setFocusPolicy(Qt::NoFocus);
        m_pGetVerCodeBT->setStyleSheet("color:#2FB3E8;");

        if(m_pVerCodeLE->text().length() == 6){
            Q_EMIT InputStateChanged(InputState::InputFinish);
            m_curInputState = InputState::InputFinish;
        }
    }
    else
    {
        m_pGetVerCodeBT->setEnabled(false);
        m_pGetVerCodeBT->setStyleSheet("color:rgba(38, 38, 38, 115);");
        if(InputState::InputFinish == m_curInputState)
        {
            m_curInputState = InputState::InputWaiting;
            Q_EMIT InputStateChanged(InputState::InputWaiting);
        }
    }
}

void InputInfos::onNetworkStateChanged(uint state)
{
    if(NM_STATE_CONNECTED_LIMIT == state)
    {
        m_nextGetVerCodeTime = 0;
        emit notGlobal();
    }
    else if(NM_STATE_CONNECTED_GLOBAL != state)
    {
        m_nextGetVerCodeTime = 0;
        emit offlineState();
    }
    else
        emit online();
}

void InputInfos::onVerCodeTextChanged(const QString &text)
{
    if(text.length() == 6 && m_pPhoneLE->text().length() == 11 && isDigitalString(m_pPhoneLE->text()))
    {
        Q_EMIT InputStateChanged(InputState::InputFinish);
        m_curInputState = InputState::InputFinish;
    }
    else if(m_curInputState == InputState::InputFinish && text.length() > 0 && text.length() < 6)
    {
        Q_EMIT InputStateChanged(InputState::InputWaiting);
        m_curInputState = InputState::InputWaiting;
    }
}

void InputInfos::clearInfos()
{
    m_pPhoneLE->clear();
    m_pVerCodeLE->clear();
    if(m_curInputState == InputState::InputFinish)
    {
        m_curInputState = InputState::InputWaiting;
        Q_EMIT InputStateChanged(m_curInputState);
    }
}

QString InputInfos::getVerificationCode()
{
    return m_pVerCodeLE->text();
}

QString InputInfos::getPhoneNumer()
{
    return m_phoneNum;
}

/**
 * @brief InputInfos::onGetVerCode
 * 获取手机验证码
 */
void InputInfos::onGetVerCode()
{
    if(!m_networkWatcher->isConnect())
    {
        emit offlineState();
        return;
    }

    m_networkWatcher->checkOnline();

    m_pGetVerCodeBT->setEnabled(false);
    m_pGetVerCodeBT->setStyleSheet("color:rgba(38, 38, 38, 115);");
    m_nextGetVerCodeTime = 60;
    m_pGetVerCodeBT->setText(tr("Recapture(60s)"));
    m_pNextGetVerCodeQTimer->start(1000);

    QDBusReply<int> reply = EduPlatformInterface::getInstance()->call("GetVerifyCode", getPhoneNumer());
    if(!reply.isValid())
    {
        qDebug() << "info: [InputInfos][onGetVerCode]: DBus Connect Failed!";
        return;
    }
    //
    if(reply.value() != 0) // 默认0为成功
    {
        qDebug() << "info: [InputInfos][onGetVerCode]: DBus request failed!";
        return;
    }
}


void InputInfos::onNextGetVerCodeTimer()
{
    if(m_nextGetVerCodeTime > 0)
        --m_nextGetVerCodeTime;
    if(m_nextGetVerCodeTime == 20 || m_nextGetVerCodeTime == 40)
        m_networkWatcher->checkOnline();
    m_pGetVerCodeBT->setText(tr("Recapture(%1s)").arg(QString::number(m_nextGetVerCodeTime)));
    if(m_nextGetVerCodeTime == 0)
    {
        m_pNextGetVerCodeQTimer->stop();
        m_pGetVerCodeBT->setEnabled(true);
        m_pGetVerCodeBT->setFocusPolicy(Qt::NoFocus);
        m_pGetVerCodeBT->setText(tr("Get code"));
        m_pGetVerCodeBT->setStyleSheet("color:#2FB3E8;");
        return;
    }

}

void InputInfos::onGetPhoneNum(int errorCode, const QString phoneNum)
{
    switch (errorCode) {
    case DBusMsgCode::No_Error:
        if(phoneNum != nullptr && phoneNum.size() == 11)
        {
            m_phoneNum = phoneNum;
            m_pPhoneLE->setText(QString(phoneNum).replace(3,4,"****"));
        } else {
            qWarning() << "error: [InputInfos] onGetPhoneNum 获取到的手机号格式错误：" << phoneNum;
        }
        break;
    case DBusMsgCode::Error_NoReply:
        m_pPhoneLE->setText(tr("Service exception..."));
        break;
    case DBusMsgCode::Error_ArgCnt:
        m_pPhoneLE->setText(tr("Invaild parameters..."));
        break;
    default:
        m_pPhoneLE->setText(tr("Unknown fault:%1").arg(errorCode));
        break;
    }

    if(DBusMsgCode::No_Error == errorCode && phoneNum != nullptr && phoneNum.size() == 11 && isDigitalString(phoneNum))
    {

    } else {

        qWarning() << "error: [InputInfos] onGetPhoneNum errorcode" << errorCode;
    }
}

void InputInfos::onClearInfo()
{
    m_pVerCodeLE->clear();
}

PhoneAuthWidget::PhoneAuthWidget(QWidget *parent)
{
    initUI();
    initConnect();
    setQSS();
}

void PhoneAuthWidget::initUI()
{
    QVBoxLayout *phoLayout = new QVBoxLayout(this);
    phoLayout->setMargin(0);

    m_pPhoTitleLB = new QLabel(this);
    m_pPhoTitleLB->setText(tr("Verification by phoneNum"));
    m_pPhoTitleLB->setProperty("class", "titleLB");
    phoLayout->addWidget(m_pPhoTitleLB, 0, Qt::AlignHCenter);

    phoLayout->addSpacing(16);
    m_pPhoPromptMsgLB = new QLabel(this);
    m_pPhoPromptMsgLB->setText(tr("「 Use SMS to verification 」"));
    m_pPhoPromptMsgLB->setProperty("class", "PromptText");
    phoLayout->addWidget(m_pPhoPromptMsgLB, 0, Qt::AlignHCenter);

    m_pPhoErrorMsgLB = new QLabel(this);
    m_pPhoErrorMsgCloneLB = new QLabel(this);
    m_pPhoErrorMsgLB->setProperty("class", "ErrorMsg");
    m_pPhoErrorMsgCloneLB->setProperty("class", "ErrorMsg");
    hidePhoneErrorMsg();
    phoLayout->addWidget(m_pPhoErrorMsgLB, 0, Qt::AlignHCenter);
    phoLayout->addWidget(m_pPhoErrorMsgCloneLB, 0, Qt::AlignHCenter);


    m_pPhoInputInfos = new InputInfos(this);
    phoLayout->addWidget(m_pPhoInputInfos, 0, Qt::AlignHCenter);

    m_pPhoSubmitBT = new QPushButton(this);
    m_pPhoSubmitBT->setFocusPolicy(Qt::NoFocus);
    m_pPhoSubmitBT->setText(tr("commit"));
    m_pPhoSubmitBT->setProperty("class", "BindBT");
    m_pPhoSubmitBT->setCursor(QCursor(Qt::PointingHandCursor));
    m_pPhoSubmitBT->setEnabled(false);
    phoLayout->addSpacing(8);
    phoLayout->addWidget(m_pPhoSubmitBT, 0, Qt::AlignHCenter);
    phoLayout->addStretch(1);

    m_pPhoPromptMsgLB->setWordWrap(true);

    m_hideTimer = new QTimer(this);
}

void PhoneAuthWidget::initConnect()
{
    connect(m_pPhoInputInfos, &InputInfos::InputStateChanged, this, &PhoneAuthWidget::onInputStateChanged);

    connect(m_pPhoSubmitBT, &QPushButton::clicked, this, &PhoneAuthWidget::onSubmitBTClick);

    connect(m_pPhoInputInfos, &InputInfos::getVerCode, this, &PhoneAuthWidget::onGetVerCode);

    connect(m_pPhoInputInfos, &InputInfos::offlineState, this, &PhoneAuthWidget::onOfflineState);

    connect(m_pPhoInputInfos, &InputInfos::notGlobal, this, &PhoneAuthWidget::onNotGlobal);

    connect(m_pPhoInputInfos, &InputInfos::online, this, &PhoneAuthWidget::hidePhoneErrorMsg);

    connect(m_hideTimer, &QTimer::timeout, this, &PhoneAuthWidget::hidePhoneErrorMsg);

    connect(this, &PhoneAuthWidget::clearInfo, m_pPhoInputInfos, &InputInfos::onClearInfo);
}

QSize PhoneAuthWidget::sizeHint() const{
    return QWidget::sizeHint();
}

void PhoneAuthWidget::hidePhoneErrorMsg()
{
    m_pPhoErrorMsgCloneLB->setFixedHeight(m_pPhoErrorMsgLB->height());
    m_pPhoErrorMsgCloneLB->show();
    m_pPhoErrorMsgLB->hide();
}

void PhoneAuthWidget::showPhoneErrorMsg()
{
    if(m_hideTimer->isActive())
        m_hideTimer->stop();

    m_pPhoErrorMsgLB->show();
    m_pPhoErrorMsgCloneLB->hide();

    //QTimer::singleShot(5*1000, this,&PhoneAuthWidget::hidePhoneErrorMsg);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->start(5*1000);
}

void PhoneAuthWidget::onInputStateChanged(InputInfos::InputState input_state)
{
    if(m_pPhoErrorMsgLB->isVisible())
        hidePhoneErrorMsg();
    if(input_state == InputInfos::InputState::InputFinish)
    {
        m_pPhoSubmitBT->setEnabled(true);
        m_pPhoSubmitBT->setFocusPolicy(Qt::NoFocus);
        m_pPhoSubmitBT->setStyleSheet("background:#2FB3E8;");
    }
    else
    {
        m_pPhoSubmitBT->setEnabled(false);
        m_pPhoSubmitBT->setStyleSheet("background:rgba(255,255,255,115);");
    }
}

void PhoneAuthWidget::onSubmitBTClick()
{
    QString tel = m_pPhoInputInfos->getPhoneNumer();
    QString verCode = m_pPhoInputInfos->getVerificationCode();

    DBusMsgCode code = EduPlatformInterface::getInstance()->CheckUserByPhone(getenv("USER"), tel, verCode);

    switch (code) {
    case DBusMsgCode::No_Error:
        Q_EMIT pageMessage(SwitchPage::SwitchToResetPWD, QList<QVariant>());
        break;
    case DBusMsgCode::Error_NetWork:
        m_pPhoErrorMsgLB->setText(tr("Network not connected~"));
        break;
    case DBusMsgCode::Error_VerifyCode_Timeout:
        m_pPhoErrorMsgLB->setText(tr("Verification Code invalid!"));
        break;
    case DBusMsgCode::Error_VerifyCodeDiff:
        m_pPhoErrorMsgLB->setText(tr("Verification Code incorrect.Please retry!"));
        break;
    case Error_ErrTimesOverMax:
        m_pPhoErrorMsgLB->setText(tr("Failed time over limit!Retry after 1 hour!"));
        break;
    default:
        qWarning() << "[ERROR] <VerificationWidget> verification error code is" << code;
        m_pPhoErrorMsgLB->setText(tr("verifaction failed!"));
        break;
    }
    if( code != DBusMsgCode::No_Error)
    {
        showPhoneErrorMsg();
        emit clearInfo();
        m_pPhoSubmitBT->setEnabled(false);
        m_pPhoSubmitBT->setStyleSheet("background:rgba(255,255,255,115);");
    }
}

void PhoneAuthWidget::onGetVerCode()
{
//    DBusMsgCode msgCode = SSOP->m_pRetrievePwdInterface->GetVerificationCode(Cfg->getUsername(), m_pPhoInputInfos->getPhoneNumer());
//    switch (msgCode) {
//    case DBusMsgCode::No_Error:
//        // 验证码获取无错误
//        break;
//        // TODO 更新错误状态
//    default:
//        break;
//    }
}

void PhoneAuthWidget::onOfflineState()
{
    m_pPhoErrorMsgLB->setText(tr("Network not connected~"));
    showPhoneErrorMsg();
}

void PhoneAuthWidget::onNotGlobal()
{
    m_pPhoErrorMsgLB->setText(tr("Network unavailable~"));
    showPhoneErrorMsg();
}

void PhoneAuthWidget::onQRCodeStateChanged(QString username, QString password, int nState)
{
    QRCodeSwepState state = static_cast<QRCodeSwepState>(nState);
    switch (state) {
    case QRCodeSwepState::WaitingSwep:
        qDebug() << "info: [QRCodePhoneAuthWidget][onQRCodeStateChanged]: waiting user swep qrcode!";
        break;
    case QRCodeSwepState::HaveSwep:
        qDebug() << "info: [QRCodePhoneAuthWidget][onQRCodeStateChanged]: user has swep code!";
        break;
    case QRCodeSwepState::CancelSwep:
        qDebug() << "info: [QRCodePhoneAuthWidget][onQRCodeStateChanged]: user cancel swep code!";
        break;
    case QRCodeSwepState::ConfirmSuccess:
    {
        qDebug() << "info: [QRCodePhoneAuthWidget][onQRCodeStateChanged]: user = [" << username << "] confirm success!";
//        if(username != Cfg->getUsername())
//        {
//            resetQRCService();
//            // TODO 更新错误信息
//        }
//        else
//            emit pageMessage(PageMessage::JumpToNewPswSet, QList<QVariant>());
//        break;
    }
    case QRCodeSwepState::QRCodeInvalid:
    {
        qDebug() << "info: [QRCodePhoneAuthWidget][onQRCodeStateChanged]: qrcode invalid!";
//        resetQRCService();
        break;
    }
    default:
        break;
    }
}

void PhoneAuthWidget::setQSS()
{
    QString style_sheet =".PromptText{"
                        "font-size:24px;"
                        "color: rgba(255,255,255,192);"
                        "}"
                        ".BindBT{"
                        "background:rgba(255,255,255,115);"
                        "width:316px;"
                        "height:64px;"
                        "border-radius:16px;"
                        "}"
                        ".ErrorMsg{"
                        "color:#FD625E;"
                        "font-size:14px;"
                        "}"
                        ".titleLB{"
                         "color:#FFFFFF;"
                         "font-size:32px;"
                         "}";
    setStyleSheet(style_sheet);
}

MyLineEdit::MyLineEdit(QWidget *parent)
{
    this->setStyleSheet("padding:10px 2px 10px 2px;"
                        "background-color:rgb(255,255,255,0);"
                        "color:grey;"
                        "font-size:16px");
    setText(tr("Verification code"));
}

void MyLineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    if(text() == tr("Verification code"))
        this->clear();
    this->setStyleSheet("padding:10px 2px 10px 2px;"
                        "background-color:rgb(255,255,255,0);"
                        "color:black;"
                        "font-size:16px");
}

void MyLineEdit::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    if(text().isEmpty())
    {
        this->setStyleSheet("padding:10px 2px 10px 2px;"
                            "background-color:rgb(255,255,255,0);"
                            "color:grey;"
                            "font-size:16px");
        setText(tr("Verification code"));
    }

}



GetPhoneNumThread::GetPhoneNumThread(QObject *parent) : QThread(parent)
{
    //获取手机号的dbus理论上也是耗时操作，故放到线程里面去请求
}

void GetPhoneNumThread::run()
{
    QString phoneNum = "";
    DBusMsgCode ret = DBusMsgCode::Error_UnknownReason;
    /**极其不良的重试策略，如果没有服务或者网络问题，会导致不停地重试,应该做优化
     * 1、需要后台服务支持，如果不存在dbus服务,应该直接报错，当前绑定失败不意味着服务不存在，判断逻辑需要研究
     * 2、监听网络变化，网络状态变化时，断网时及时提醒用户网络未连接；联网时，如未获取到已绑定的手机号，需要重新获取；
     * 网络异常时，固定次数的重试，都失败后提示用户网络请求异常，并提供可手动刷新的策略
     * */
    while (DBusMsgCode::No_Error != ret)
    {
        ret = AccountsInterface::getInstance()->GetUserPhone(getenv("USER"), phoneNum);
        Q_EMIT resultReady(ret, phoneNum);
        sleep(5);
    }
}


bool isDigitalString(QString s)
{
    for(int i = 0; i< s.length(); i++)
    {
        if(s.at(i) > '9' || s.at(i) < '0')
        {
            qDebug() << "phoneNum error at" << i+1 << "position";
            return false;
        }
        if(i == 2)
            i += 4;
    }

    for(int i = 3; i< 7; i++)
    {
        if(s.at(i) != '*')
        {
            qDebug() << "phoneNum error at" << i+1 << "position  unencrypted";
            return false;
        }
    }
    return true;
}
