#include "lockwidget.h"
#include "ui_lockwidget.h"

#include <QDateTime>
#include <QTimer>
#include <QDebug>

#include "authdialog.h"
#include "virtualkeyboard.h"

LockWidget::LockWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::LockWidget)
{
    ui->setupUi(this);
    initUI();

    authDialog = new AuthDialog(this);
    connect(authDialog, &AuthDialog::authenticateCompete,
            this, &LockWidget::closed);
    connect(this, &LockWidget::capsLockChanged,
            authDialog, &AuthDialog::onCapsLockChanged);
    setFocusProxy(authDialog);
}

LockWidget::~LockWidget()
{
    delete ui;
}

void LockWidget::startAuth()
{
    if(authDialog)
    {
        authDialog->startAuth();
    }
}

void LockWidget::stopAuth()
{
    if(authDialog)
    {
        authDialog->stopAuth();
    }
}

void LockWidget::initUI()
{
    //显示系统时间
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&]{
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        ui->lblTime->setText(time);
    });

    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->lblTime->setText(time);
    ui->lblTime->setStyleSheet("QLabel{color:white; font-size: 55px;}");
    ui->lblTime->adjustSize();
    timer->start(1000);

    QString date = QDate::currentDate().toString("yyyy/MM/dd dddd");
    qDebug() << "current date: " << date;
    ui->lblDate->setText(date);
    ui->lblDate->setStyleSheet("QLabel{color:white; font-size: 20px;}");
    ui->lblDate->adjustSize();

    //虚拟键盘
    vKeyboard = new VirtualKeyboard(this);
    vKeyboard->hide();
    connect(vKeyboard, &VirtualKeyboard::aboutToClose, vKeyboard, &VirtualKeyboard::hide);

    ui->btnKeyboard->setIcon(QIcon(":/image/assets/keyboard.png"));
    ui->btnKeyboard->setFixedSize(39, 39);
    ui->btnKeyboard->setIconSize(QSize(39, 39));
    ui->btnKeyboard->setFocusPolicy(Qt::NoFocus);
    connect(ui->btnKeyboard, &QPushButton::clicked, vKeyboard, &VirtualKeyboard::show);
}

/* lockscreen follows cursor */
void LockWidget::resizeEvent(QResizeEvent */*event*/)
{
    //认证窗口
    authDialog->setGeometry((width()-authDialog->geometry().width())/2, 0,
                            authDialog->width(), height());

    //系统时间
    ui->widgetTime->move(0, height() - 150);

    //虚拟键盘按钮
    ui->btnKeyboard->move(width() - 100, height() - 100);
}
