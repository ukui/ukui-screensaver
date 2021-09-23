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
#include <QScrollArea>
#include <QScrollBar>
#include <QMenu>
#include <QtX11Extras/QX11Info>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <QGSettings>
#include "authdialog.h"
#include "virtualkeyboard.h"
#include "users.h"
#include "displaymanager.h"
#include "config.h"
#include "commonfunc.h"

#define TIME_TYPE_SCHEMA "org.ukui.control-center.panel.plugins"
float scale;
LockWidget::LockWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::LockWidget),
      usersMenu(nullptr),
      scrollArea(nullptr),
      users(new Users(this)),
      displayManager(new DisplayManager(this)),
      timeType(24)
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

    if(users->getUsers().count() < 2){
        ui->btnSwitchUser->hide();
    }
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

        if(scrollArea && scrollArea->isVisible()){
            scrollArea->hide();
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

void LockWidget::setX11Focus()
{
    if(authDialog){
    	authDialog->setX11Focus();
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
        if (keys.contains("date")) {
            dateType = time_type->get("date").toString();
        }
    }

    //显示系统时间
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&]{
        if(timeType == 12)
            ui->lblTime->setText(QDateTime::currentDateTime().toString("A hh:mm"));
        else
            ui->lblTime->setText(QDateTime::currentDateTime().toString("hh:mm"));

        if(dateType == "cn")
            ui->lblDate->setText(QDate::currentDate().toString("yyyy/MM/dd ddd"));
        else
            ui->lblDate->setText(QDate::currentDate().toString("yyyy-MM-dd ddd"));
    });

    if(timeType == 12)
    	ui->lblTime->setText(QDateTime::currentDateTime().toString("A hh:mm"));
    else
        ui->lblTime->setText(QDateTime::currentDateTime().toString("hh:mm"));

    ui->lblTime->setStyleSheet("QLabel{color:white; font-size: 50px;}");
    ui->lblTime->setAlignment(Qt::AlignCenter);
    ui->lblTime->adjustSize();
    timer->start(1000);

//    QString date = QDate::currentDate().toString("yyyy/MM/dd ddd");
//    qDebug() << "current date: " << date;
//    ui->lblDate->setText(date);
    if(dateType == "cn")
        ui->lblDate->setText(QDate::currentDate().toString("yyyy/MM/dd ddd"));
    else
        ui->lblDate->setText(QDate::currentDate().toString("yyyy-MM-dd ddd"));

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
#ifndef USE_INTEL
        if(width() < 1280)
            powermanager->showSmallSize();
        else
            powermanager->showNormalSize();
#endif
        powermanager->setGeometry((width()-powermanager->width())/2,
                                  (height()-powermanager->height())/2,
                                  powermanager->width(),powermanager->height());
    }
}

void LockWidget::switchToGreeter()
{
    if(authDialog)
    {
        authDialog->stopAuth();
    }
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

    scrollArea = new QScrollArea(this);
//    scrollArea->setAttribute(Qt::WA_TranslucentBackground); //设置背景透明
//    scrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground); //设置背景透明
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //屏蔽水平滚动条
    scrollArea->setStyleSheet("QScrollArea {background-color:transparent;}");
    scrollArea->viewport()->setStyleSheet("background-color:transparent;");
    scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    scrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    scrollContents = new QWidget(scrollArea);
    scrollArea->setWidget(scrollContents);
    scrollArea->hide();

    if(!usersMenu)
    {
        usersMenu = new QMenu(scrollContents);
        usersMenu->setObjectName("usersMenu");
        usersMenu->setToolTipsVisible(true);
        usersMenu->setStyleSheet("background-color: rgba(255,255,255,20%);\
                                 color: white; \
                                 border-radius: 4px; \
                                 padding: 5px 5px 5px 5px;");
	//如果没有设置x11属性，则由于弹出菜单受窗口管理器管理，而主窗口不受，在点击菜单又点回主窗口会闪屏。
        usersMenu->setWindowFlags(Qt::X11BypassWindowManagerHint);
        //usersMenu->hide();
        connect(usersMenu, &QMenu::triggered,
                this, &LockWidget::onUserMenuTrigged);
        connect(ui->btnSwitchUser, &QPushButton::clicked,
                this, [&]{
                if(scrollArea->isVisible()){
                    scrollArea->hide();
                }
                else{
                    scrollArea->show();
                   // usersMenu->setActiveAction(nullptr);
                    scrollArea->setFocus();
                }
        });

    }

    if(displayManager->getDisplayType() == "gdm"){
        QAction *action = new QAction(QIcon(users->getDefaultIcon()),
                                      tr("SwitchUser"), this);
        action->setToolTip("SwitchUser");
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
            action->setToolTip("Guest");
            usersMenu->addAction(action);
        }
    }

    scrollContents->setFixedSize(usersMenu->size());
    //scrollArea->setFixedSize(scrollContents->width(),scrollContents->height()+20);
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
    authDialog->setGeometry((width()-authDialog->geometry().width())/2,height()/3, \
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
    if(scrollArea){
        if(scrollContents->height() < height()/2){
            scrollArea->setFixedSize(scrollContents->width(),scrollContents->height()+10);
        }else{
            scrollArea->setFixedSize(scrollContents->width(),height()/2);
        }

        scrollArea->move(width() - x, \
                    height() - y - scrollArea->height() - 5);
    }
    
    if(powermanager){
#ifndef USE_INTEL
        if(width() < 1280)
            powermanager->showSmallSize();
        else
            powermanager->showNormalSize();
#endif
        powermanager->setGeometry((width()- powermanager->width())/2,
                                  (height()-powermanager->height())/2,
                                  powermanager->width(),powermanager->height());

    }
    XSetInputFocus(QX11Info::display(),this->winId(),RevertToParent,CurrentTime);
}


void LockWidget::onUserAdded(const UserItem &user)
{
    QAction *action = new QAction(QIcon(user.icon), user.realName, this);

    QFont font;
    font.setPixelSize(16);
    QString str = ElideText(font,120,user.realName);
    if(user.realName != str)
         action->setToolTip(user.realName);
    action->setData(user.realName);
    action->setText(str);
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

    if(authDialog)
    {
        authDialog->stopAuth();
    }

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
}
