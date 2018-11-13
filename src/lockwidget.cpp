#include "lockwidget.h"
#include "ui_lockwidget.h"

#include <QDateTime>
#include <QTimer>
#include <QDebug>
#include <QMenu>

#include "authdialog.h"
#include "virtualkeyboard.h"
#include "users.h"
#include "displaymanager.h"

LockWidget::LockWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::LockWidget),
      usersMenu(nullptr),
      users(new Users(this)),
      displayManager(new DisplayManager(this))
{
    ui->setupUi(this);

    UserItem user = users->getUserByName(getenv("USER"));
    authDialog = new AuthDialog(user, this);
    connect(authDialog, &AuthDialog::authenticateCompete,
            this, &LockWidget::closed);
    connect(this, &LockWidget::capsLockChanged,
            authDialog, &AuthDialog::onCapsLockChanged);
    initUI();
}

LockWidget::~LockWidget()
{
    delete ui;
}

void LockWidget::closeEvent(QCloseEvent *event)
{
    qDebug() << "LockWidget::closeEvent";
    authDialog->close();
    return QWidget::closeEvent(event);
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
    setFocusProxy(authDialog);

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
    connect(vKeyboard, &VirtualKeyboard::aboutToClose,
            vKeyboard, &VirtualKeyboard::hide);

    ui->btnKeyboard->setIcon(QIcon(":/image/assets/keyboard.png"));
    ui->btnKeyboard->setFixedSize(39, 39);
    ui->btnKeyboard->setIconSize(QSize(39, 39));
    connect(ui->btnKeyboard, &QPushButton::clicked,
            this, [&]{
        qDebug() << vKeyboard->isHidden();
        vKeyboard->setVisible(vKeyboard->isHidden());
    });

    //用户切换
    if(displayManager->canSwitch())
    {
        initUserMenu();
    }
}

void LockWidget::initUserMenu()
{
    ui->btnSwitchUser->setIcon(QIcon(":/image/assets/avatar.png"));
    ui->btnSwitchUser->setIconSize(QSize(39, 39));
    ui->btnSwitchUser->setFixedSize(39, 39);

    if(!usersMenu)
    {
        usersMenu = new QMenu(this);
        ui->btnSwitchUser->setMenu(usersMenu);
        connect(usersMenu, &QMenu::triggered,
                this, &LockWidget::onUserMenuTrigged);
    }

    connect(users, &Users::userAdded, this, &LockWidget::onUserAdded);
    connect(users, &Users::userDeleted, this, &LockWidget::onUserDeleted);

    for(auto user : users->getUsers())
    {
        onUserAdded(user);
    }

    if(displayManager->hasGuestAccount())
    {
        QAction *action = new QAction(QIcon(users->getDefaultIcon()),
                                      tr("Guest"), this);
        action->setData("Guest");
        usersMenu->addAction(action);
    }

    {
        QAction *action = new QAction(QIcon(users->getDefaultIcon()),
                                      tr("SwitchUser"), this);
        action->setData("SwitchUser");
        usersMenu->addAction(action);
    }

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
    ui->btnKeyboard->move(width() - 60, 20);

    ui->btnSwitchUser->move(width() - 120, 20);
}


void LockWidget::onUserAdded(const UserItem &user)
{
    QAction *action = new QAction(QIcon(user.icon), user.realName, this);
    action->setData(user.name);
    usersMenu->addAction(action);
}

void LockWidget::onUserDeleted(const UserItem &user)
{
    for(auto action : usersMenu->actions())
    {
        if(action->data() == user.name)
            usersMenu->removeAction(action);
    }
}

void LockWidget::onUserMenuTrigged(QAction *action)
{
    qDebug() << action->data().toString() << "selected";

    QString userName = action->data().toString();
    if(userName == "Guest")
    {
        displayManager->switchToGuest();
    }
    else if(userName == "SwitchUser")
    {
        displayManager->switchToGreeter();
    }
    else
    {
        displayManager->switchToUser(userName);
    }
    if(authDialog)
    {
        authDialog->stopAuth();
    }
}
