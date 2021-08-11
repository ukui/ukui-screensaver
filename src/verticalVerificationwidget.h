#ifndef VERTICALVERIFICATIONWIDGET_H
#define VERTICALVERIFICATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <verificationwidget.h>

#include "common.h"

class VerticalVerificationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VerticalVerificationWidget(QWidget *parent = nullptr);
    ~VerticalVerificationWidget();

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

#endif // VERTICALVERIFICATIONWIDGET_H
