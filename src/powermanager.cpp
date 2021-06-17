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

#include <QLabel>
#include <QVBoxLayout>
#include <QDBusReply>
#include <QPixmap>
#include <QListWidgetItem>
#include <QListWidget>
#include <QDebug>
#include <QDBusInterface>
#include "powermanager.h"

const static QString login1Service = QStringLiteral("org.freedesktop.login1");
const static QString login1Path = QStringLiteral("/org/freedesktop/login1");
const static QString login1ManagerInterface = QStringLiteral("org.freedesktop.login1.Manager");

PowerManager::PowerManager(QWidget *parent)
    : QListWidget(parent),
      lasttime(QTime::currentTime())
{
    resize(ITEM_WIDTH*7, ITEM_HEIGHT);
    setFlow(QListWidget::LeftToRight);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionMode(QListWidget::NoSelection);

    QObject::connect(this,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(powerClicked(QListWidgetItem*)));

    sessionInterface = new QDBusInterface("org.gnome.SessionManager",
                                          "/org/gnome/SessionManager",
                                          "org.gnome.SessionManager",
                                          QDBusConnection::sessionBus(),
                                          this);

    loginInterface = new QDBusInterface(login1Service,
                                        login1Path,
                                        login1ManagerInterface,
                                        QDBusConnection::systemBus(),
                                        this);

    m_count = 5;
    QDBusReply<QString> stateReply = loginInterface->call("CanSuspend");
    if(stateReply.isValid() && stateReply.value() == "yes"){
        canSuspend = true;
        m_count ++;
    }else{
        canSuspend = false;
    }

    QDBusReply<QString> stateReply1 = loginInterface->call("CanHibernate");
    if(stateReply1.isValid() && stateReply1.value() == "yes"){
        canHibernate = true;
        m_count ++;
    }else{
        canHibernate = false;
    }

    initUI();
    resize(ITEM_WIDTH*m_count, ITEM_HEIGHT);
}

void PowerManager::powerClicked(QListWidgetItem *item)
{
    int interval = lasttime.msecsTo(QTime::currentTime());
    if(interval < 200 && interval > -200)
        return ;
    lasttime = QTime::currentTime();

    QString name = itemWidget(item)->objectName();
    if(name == lockWidget->objectName())
        lockWidgetClicked();
    else if(name == switchWidget->objectName())
        switchWidgetClicked();
    else if(name == logoutWidget->objectName())
        logoutWidgetCliced();
    else if(name == rebootWidget->objectName())
        rebootWidgetClicked();
    else if(name == shutdownWidget->objectName())
        shutdownWidgetClicked();
    else if(suspendWidget &&  name == suspendWidget->objectName())
        suspendWidgetClicked();
    else if(hibernateWidget && name == hibernateWidget->objectName())
        hibernateWidgetClicked();
}

void PowerManager::lockWidgetClicked()
{
    emit lock();
}

void PowerManager::switchWidgetClicked()
{
    emit switchToUser();
}

void PowerManager::suspendWidgetClicked()
{
    loginInterface->call("Suspend",true);
    emit lock();
}

void PowerManager::hibernateWidgetClicked()
{
    loginInterface->call("Hibernate",true);
    emit lock();
}
void PowerManager::shutdownWidgetClicked()
{
    sessionInterface->call("powerOff");
}

void PowerManager::rebootWidgetClicked()
{
    sessionInterface->call("reboot");
}

void PowerManager::logoutWidgetCliced()
{
    sessionInterface->call("logout");
}

void PowerManager::showSmallSize()
{
    for(int i = 0;i<count();i++){
        QListWidgetItem *item = this->item(i);
        item->setSizeHint(QSize(ITEM_WIDTH*0.8,ITEM_HEIGHT));
        itemWidget(item)->setFixedSize(ITEM_WIDTH*0.8,ITEM_HEIGHT);
    }
    resize(ITEM_WIDTH*m_count*0.8,ITEM_HEIGHT);
}

void PowerManager::showNormalSize()
{
    for(int i = 0;i<count();i++){
        QListWidgetItem *item = this->item(i);
        item->setSizeHint(QSize(ITEM_WIDTH,ITEM_HEIGHT));
        itemWidget(item)->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
    }
    resize(ITEM_WIDTH*m_count,ITEM_HEIGHT);
}

void PowerManager::initUI()
{

    lockWidget = new QWidget(this);
    lockWidget->setObjectName("lockWidget");
    QLabel *lockFace = new QLabel(this);
    QLabel *lockLabel = new QLabel(this);
    lockFace->setAlignment(Qt::AlignCenter);
    lockLabel->setAlignment(Qt::AlignCenter);
    lockFace->setPixmap(QPixmap(":/image/assets/lock.png").scaled(58,58));
    lockLabel->setText(tr("Lock Screen"));

    lockWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
    QVBoxLayout *locklayout = new QVBoxLayout(lockWidget);
    locklayout->addWidget(lockFace);
    locklayout->addWidget(lockLabel);
    lockWidget->installEventFilter(this);

    switchWidget = new QWidget(this);
    switchWidget->setObjectName("switchWidget");
    QLabel *switchFace = new QLabel(this);
    QLabel *switchLabel =  new QLabel(this);
    switchFace->setAlignment(Qt::AlignCenter);
    switchLabel->setAlignment(Qt::AlignCenter);
    switchFace->setPixmap(QPixmap(":/image/assets/switchGreeter.png").scaled(58,58));
    switchLabel->setText(tr("Switch User"));
    switchWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
    QVBoxLayout *switchlayout = new QVBoxLayout(switchWidget);
    switchlayout->addWidget(switchFace);
    switchlayout->addWidget(switchLabel);
    switchWidget->installEventFilter(this);

    logoutWidget = new QWidget(this);
    logoutWidget->setObjectName("logoutWidget");
    QLabel *logoutFace = new QLabel(this);
    QLabel *logoutLabel = new QLabel(this);
    logoutFace->setAlignment(Qt::AlignCenter);
    logoutLabel->setAlignment(Qt::AlignCenter);
    logoutFace->setPixmap(QPixmap(":/image/assets/logout.png").scaled(48,48));
    logoutLabel->setText(tr("Log Out"));
    logoutWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
    QVBoxLayout *logoutlayout = new QVBoxLayout(logoutWidget);
    logoutlayout->addWidget(logoutFace);
    logoutlayout->addWidget(logoutLabel);
    logoutWidget->installEventFilter(this);

    rebootWidget = new QWidget(this);
    rebootWidget->setObjectName("rebootWidget");
    QLabel *rebootFace = new QLabel(this);
    QLabel *rebootLabel = new QLabel(this);
    rebootFace->setAlignment(Qt::AlignCenter);
    rebootLabel->setAlignment(Qt::AlignCenter);
    rebootFace->setPixmap(QPixmap(":/image/assets/reboot.png").scaled(58,58));
    rebootLabel->setText(tr("Restart"));
    rebootWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
    QVBoxLayout *rebootlayout = new QVBoxLayout(rebootWidget);
    rebootlayout->addWidget(rebootFace);
    rebootlayout->addWidget(rebootLabel);
    rebootWidget->installEventFilter(this);

    shutdownWidget = new QWidget(this);
    shutdownWidget->setObjectName("shutdownWidget");
    QLabel *shutdownFace  = new QLabel(this);
    QLabel *shutdownLabel = new QLabel(this);
    shutdownLabel->setAlignment(Qt::AlignCenter);
    shutdownFace->setAlignment(Qt::AlignCenter);
    shutdownFace->setPixmap(QPixmap(":/image/assets/shutdown.png").scaled(58,58));
    shutdownLabel->setText(tr("Power Off"));
    shutdownWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
    QVBoxLayout *shutdownlayout = new QVBoxLayout(shutdownWidget);
    shutdownlayout->addWidget(shutdownFace);
    shutdownlayout->addWidget(shutdownLabel);
    shutdownWidget->installEventFilter(this);

    QListWidgetItem *item0 = new QListWidgetItem();
    item0->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(this->count(), item0);
    setItemWidget(item0, lockWidget);

    QListWidgetItem *item1 = new QListWidgetItem();
    item1->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(this->count(), item1);
    setItemWidget(item1, switchWidget);

    QListWidgetItem *item2 = new QListWidgetItem();
    item2->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(this->count(), item2);
    setItemWidget(item2, logoutWidget);

    QListWidgetItem *item3 = new QListWidgetItem();
    item3->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(this->count(), item3);
    setItemWidget(item3, rebootWidget);

    QListWidgetItem *item4 = new QListWidgetItem();
    item4->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(this->count(), item4);
    setItemWidget(item4, shutdownWidget);

    suspendWidget = nullptr;
    if(canSuspend){
        suspendWidget = new QWidget(this);
        suspendWidget->setObjectName("suspendWidget");
        QLabel *suspendFace  = new QLabel(this);
        QLabel *suspendLabel = new QLabel(this);
        suspendLabel->setAlignment(Qt::AlignCenter);
        suspendFace->setAlignment(Qt::AlignCenter);
        suspendFace->setPixmap(QPixmap(":/image/assets/suspend.png").scaled(48,48));
        suspendLabel->setText(tr("Suspend"));
        suspendWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
        QVBoxLayout *suspendlayout = new QVBoxLayout(suspendWidget);
        suspendlayout->addWidget(suspendFace);
        suspendlayout->addWidget(suspendLabel);
        suspendWidget->installEventFilter(this);

        QListWidgetItem *item5 = new QListWidgetItem();
        item5->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        insertItem(this->count(), item5);
        setItemWidget(item5, suspendWidget);
    }

    hibernateWidget = nullptr;
    if(canHibernate){
        hibernateWidget = new QWidget(this);
        hibernateWidget->setObjectName("hibernateWidget");
        QLabel *hibernateFace  = new QLabel(this);
        QLabel *hibernateLabel = new QLabel(this);
        hibernateLabel->setAlignment(Qt::AlignCenter);
        hibernateFace->setAlignment(Qt::AlignCenter);
        hibernateFace->setPixmap(QPixmap(":/image/assets/hibernate.png").scaled(48,48));
        hibernateLabel->setText(tr("Sleep"));
        hibernateWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
        QVBoxLayout *hibernatelayout = new QVBoxLayout(hibernateWidget);
        hibernatelayout->addWidget(hibernateFace);
        hibernatelayout->addWidget(hibernateLabel);
        hibernateWidget->installEventFilter(this);

        QListWidgetItem *item6 = new QListWidgetItem();
        item6->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        insertItem(6, item6);
        setItemWidget(item6, hibernateWidget);
    }
}
