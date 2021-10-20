#ifndef VERIFICATIONWIDGET_H
#define VERIFICATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include "wechatauthdialog.h"
#include "common.h"
#include "networkwatcher.h"

class InputInfos;
class PhoneAuthWidget;
class MyLineEdit;

class InputInfos : public QWidget
{
    Q_OBJECT
public:
    enum InputState{
        InputWaiting
        , InputFinish
    };
    InputInfos(QWidget *parent = nullptr);
    QString getPhoneNumer();
    QString getVerificationCode();
    void clearInfos();

Q_SIGNALS:
    void getVerCode();
    void InputStateChanged(InputState input_state);
    void offlineState();
    void notGlobal();
    void online();

protected:
    void paintEvent(QPaintEvent *);

private Q_SLOTS:
    void onPhoneTextChanged(const QString &text);
    void onVerCodeTextChanged(const QString &text);
    void onGetVerCode();
    void onNextGetVerCodeTimer();
    void onGetPhoneNum(int errorCode, const QString phoneNum);
    void onNetworkStateChanged(uint state);

public Q_SLOTS:
    void onClearInfo();

private:
    void initUI();
    void initConnect();
    void setQSS();

private:
    QLabel *m_pPhoneIconLB;
    QLineEdit *m_pPhoneLE;
    QLabel *m_pVerCodeIconLB;
    MyLineEdit *m_pVerCodeLE;
    QPushButton *m_pGetVerCodeBT;
    QTimer *m_pNextGetVerCodeQTimer;
    int m_nextGetVerCodeTime;
    QString m_phoneNum = nullptr;

    InputState m_curInputState;
    NetWorkWatcher *m_networkWatcher;
};

class VerificationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VerificationWidget(QWidget *parent = nullptr);
    ~VerificationWidget();

    void reloadQR();

Q_SIGNALS:
    void pageMessage(SwitchPage id, QList<QVariant> argvs);
private:
    QLabel *m_titleLB;
    QLabel *m_phonePromptMsgLB;
    WeChatAuthDialog *m_weChatWidget;
//public:
    PhoneAuthWidget *m_phoneAuthWidget;

protected:
    void paintEvent(QPaintEvent *);
};

class PhoneAuthWidget : public QWidget
{
    Q_OBJECT
public:
    PhoneAuthWidget(QWidget* parent = nullptr);
    QSize sizeHint() const;

private:
    void initUI();
    void initConnect();
    void setQSS();

    void showPhoneErrorMsg();
    void hidePhoneErrorMsg();
Q_SIGNALS:
    void pageMessage(SwitchPage id, QList<QVariant> argvs);
    void clearInfo();
private Q_SLOTS:
    void onInputStateChanged(InputInfos::InputState input_state);
    void onSubmitBTClick();
    void onGetVerCode();
    void onQRCodeStateChanged(QString username, QString password, int nState);
    void onOfflineState();
    void onNotGlobal();


private:
    // 手机号验证
    QLabel *m_pPhoTitleLB;
    QLabel *m_pPhoPromptMsgLB;
    QLabel *m_pPhoErrorMsgLB;
    QLabel *m_pPhoErrorMsgCloneLB; // 控制格式
    QPushButton *m_pPhoSubmitBT;
    InputInfos *m_pPhoInputInfos;
    QTimer *m_hideTimer;
};

class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEdit(QWidget *parent = nullptr);

/*public Q_SLOTS:
    void onTextChanged(const QString &text);*/

protected:
    void focusInEvent(QFocusEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
};

class GetPhoneNumThread : public QThread
{
    Q_OBJECT
public:
    GetPhoneNumThread(QObject *parent);

protected:
    void run() override;

Q_SIGNALS:
    void resultReady(int errorCode, const QString phoneNum);
};
#endif // VERIFICATIONWIDGET_H
