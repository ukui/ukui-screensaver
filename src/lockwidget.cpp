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
#include "lockwidget.h"
#include "powermanager.h"
#include "ui_lockwidget.h"

#include <QDateTime>
#include <QTimer>
#include <QDebug>
#include <QMenu>
#include <QtX11Extras/QX11Info>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <QGSettings>
#include "authdialog.h"
#include "virtualkeyboard.h"
#include "users.h"
#include "displaymanager.h"

#define TIME_TYPE_SCHEMA "org.ukui.control-center.panel.plugins"
float scale;
LockWidget::LockWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::LockWidget),
      usersMenu(nullptr),
      users(new Users(this)),
      timeType(24),
      displayManager(new DisplayManager(this))
{
    scale = 1.0;
    ui->setupUi(this);

    UserItem user = users->getUserByName(getenv("USER"));
    authDialog = new AuthDialog(user, this);
    authDialog->installEventFilter(this);
    connect(authDialog, &AuthDialog::authenticateCompete,
            this, &LockWidget::closed);
    connect(this, &LockWidget::capsLockChanged,
            authDialog, &AuthDialog::onCapsLockChanged);
    this->installEventFilter(this);
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

bool LockWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == 2){
        if(obj == ui->btnPowerManager || obj == ui->btnSwitchUser)
            return false;

        if(usersMenu && usersMenu->isVisible()){
            usersMenu->hide();
	    }
 	    if(powermanager->isVisible()){
        	authDialog->setFocus();
        	authDialog->show();
        	powermanager->hide();
        }
    }

    return false;
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

    if(QGSettings::isSchemaInstalled(TIME_TYPE_SCHEMA)){
    	QGSettings *time_type = new QGSettings(TIME_TYPE_SCHEMA);
        QStringList keys = time_type->keys();
    	if (keys.contains("hoursystem")) {
        	timeType = time_type->get("hoursystem").toInt();
	}
    }

    //显示系统时间
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&]{
	if(timeType == 12)
            ui->lblTime->setText(QDateTime::currentDateTime().toString("ap hh:mm"));
        else
	    ui->lblTime->setText(QDateTime::currentDateTime().toString("hh:mm"));
	QString date = QDate::currentDate().toString("yyyy/MM/dd ddd");
	ui->lblDate->setText(date);
    });

    if(timeType == 12)
    	ui->lblTime->setText(QDateTime::currentDateTime().toString("ap hh:mm"));
    else
	ui->lblTime->setText(QDateTime::currentDateTime().toString("hh:mm"));

    ui->lblTime->setStyleSheet("QLabel{color:white; font-size: 50px;}");
    ui->lblTime->setAlignment(Qt::AlignCenter);
    ui->lblTime->adjustSize();
    timer->start(1000);

    QString date = QDate::currentDate().toString("yyyy/MM/dd ddd");
    qDebug() << "current date: " << date;
    ui->lblDate->setText(date);
    ui->lblDate->setStyleSheet("QLabel{color:white; font-size: 16px;}");
    ui->lblDate->setAlignment(Qt::AlignCenter);
    ui->lblDate->adjustSize();
    ui->widgetTime->adjustSize();

    //电源管理
    ui->btnPowerManager->setIcon(QIcon(":/image/assets/powerManager.png"));
    ui->btnPowerManager->setFixedSize(52,48);
    ui->btnPowerManager->setIconSize(QSize(30,30));
    ui->btnPowerManager->setFocusPolicy(Qt::NoFocus);
    ui->btnPowerManager->installEventFilter(this);

    connect(ui->btnPowerManager,&QPushButton::clicked
        ,this,&LockWidget::showPowerManager);

    powermanager = new PowerManager(this);
    powermanager->hide();
    connect(powermanager,SIGNAL(lock())
            ,this,SLOT(showPowerManager()));
    connect(powermanager,SIGNAL(switchToUser())
            ,this,SLOT(switchToGreeter()));

    //虚拟键盘
    vKeyboard = new VirtualKeyboard(this);
    vKeyboard->hide();

    connect(vKeyboard, &VirtualKeyboard::aboutToClose,
            vKeyboard, &VirtualKeyboard::hide);

    ui->btnKeyboard->setIcon(QIcon(":/image/assets/keyboard.png"));
    ui->btnKeyboard->setFixedSize(52, 48);
    ui->btnKeyboard->setIconSize(QSize(30, 30));
    ui->btnKeyboard->setFocusPolicy(Qt::NoFocus);
    ui->btnKeyboard->installEventFilter(this);
/*    connect(ui->btnKeyboard, &QPushButton::clicked,
            this, [&]{
        qDebug() << vKeyboard->isHidden();
        vKeyboard->setVisible(vKeyboard->isHidden());
    });
*/
    connect(ui->btnKeyboard, &QPushButton::clicked,
            this, &LockWidget::showVirtualKeyboard);

    //用户切换
    if(displayManager->canSwitch())
    {
        initUserMenu();
    }
}

void LockWidget::showVirtualKeyboard()
{
    vKeyboard->setVisible(vKeyboard->isHidden());
    setVirkeyboardPos();
}

void LockWidget::showPowerManager()
{
    if(powermanager->isVisible()){
        authDialog->setFocus();
        authDialog->show();
        powermanager->hide();
    }
    else{     
        authDialog->hide();
        powermanager->show();
        powermanager->setGeometry((width()-ITEM_WIDTH*5)/2,
                                  (height()-ITEM_HEIGHT)/2,
                                  ITEM_WIDTH*5,ITEM_HEIGHT);
    }
}

void LockWidget::switchToGreeter()
{
    displayManager->switchToGreeter();;
}

void LockWidget::setVirkeyboardPos()
{
    if(vKeyboard)
    {
        vKeyboard->setGeometry(0,
                               height() - height()/3,
                               width(), height()/3);

    }
}


void LockWidget::initUserMenu()
{
    ui->btnSwitchUser->setIcon(QIcon(":/image/assets/switchUser.png"));
    ui->btnSwitchUser->setIconSize(QSize(36, 24));
    ui->btnSwitchUser->setFixedSize(52, 48);
    ui->btnSwitchUser->setFocusPolicy(Qt::NoFocus);
    if(!usersMenu)
    {
        usersMenu = new QMenu(this);
        usersMenu->setObjectName("usersMenu");
/*
* qt5.6上，qmenu文字以图标左方为起点，20.04上文字以图标右方为起点，所以
* qt5.6时，左边距要设置大一点,避免与图标重合
*/
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
        usersMenu->setStyleSheet("QMenu::item{padding: 2px 10px 2px 10px;}");
#else
        usersMenu->setStyleSheet("QMenu::item{padding: 2px 10px 2px 30px;}");
#endif

	//如果没有设置x11属性，则由于弹出菜单受窗口管理器管理，而主窗口不受，在点击菜单又点回主窗口会闪屏。
        usersMenu->setWindowFlags(Qt::X11BypassWindowManagerHint);
        usersMenu->hide();
        connect(usersMenu, &QMenu::triggered,
                this, &LockWidget::onUserMenuTrigged);
        connect(ui->btnSwitchUser, &QPushButton::clicked,
                this, [&]{
                if(usersMenu->isVisible()){
                    usersMenu->hide();
		}
                else{
                    usersMenu->show();
                    usersMenu->setActiveAction(nullptr);
                    usersMenu->setFocus();
                }
        });

    }

    if(displayManager->getDisplayType() == "gdm"){
        QAction *action = new QAction(QIcon(users->getDefaultIcon()),
                                      tr("SwitchUser"), this);
        action->setData("SwitchUser");
        usersMenu->addAction(action);
    }

    else if(displayManager->getDisplayType() == "lightdm"){
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
    }
}

void LockWidget::keyReleaseEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_K){
        if(e->modifiers() & Qt::ControlModifier)
            showVirtualKeyboard();
    }
}

/* lockscreen follows cursor */
void LockWidget::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
        //重新计算缩放比例
    scale = QString::number(size.width() / 1920.0, 'f', 1).toFloat();

    if(scale > 1)
            scale = 1;

    //系统时间
    ui->widgetTime->move((width()-ui->widgetTime->geometry().width())/2, 59*scale);

    //认证窗口
    //设置认证窗口左右居中
    authDialog->setGeometry((width()-authDialog->geometry().width())/2,ui->widgetTime->geometry().bottom() + 176*scale,
                            authDialog->width(), height());

    //右下角按钮,x,y的初始值代表距离右下角的距离。
    int x=19,y=86;
    x = x + ui->btnPowerManager->width();
    ui->btnPowerManager->move(width() - x,height() - y);

    x = x+ui->btnKeyboard->width();
    ui->btnKeyboard->move(width() - x, height() -  y);

    x = x + ui->btnSwitchUser->width();
    ui->btnSwitchUser->move(width() - x, height() - y);
    setVirkeyboardPos();

    //设置弹出菜单，设置弹出菜单的坐标为切换用户按钮的上方，中间保持一定间隔。
    if(usersMenu){
   	 usersMenu->move(width() - x , \
                    height() - y - usersMenu->height() - 5);
    }

    XSetInputFocus(QX11Info::display(),this->winId(),RevertToParent,CurrentTime);
}


void LockWidget::onUserAdded(const UserItem &user)
{
    QAction *action = new QAction(QIcon(user.icon), user.realName, this);
    action->setData(user.name);
    usersMenu->addAction(action);
    usersMenu->adjustSize();
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
