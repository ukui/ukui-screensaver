#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QListWidgetItem>
#include <QListWidget>
#include <QDebug>
#include <QDBusInterface>
#include "powermanager.h"

PowerManager::PowerManager(QWidget *parent)
 : QListWidget(parent),
   lasttime(QTime::currentTime())
{

    resize(ITEM_WIDTH*5, ITEM_HEIGHT);
    setFlow(QListWidget::LeftToRight);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionMode(QListWidget::NoSelection);

    QObject::connect(this,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(powerClicked(QListWidgetItem*)));
    initUI();

}

void PowerManager::powerClicked(QListWidgetItem *item)
{
    int interval = lasttime.msecsTo(QTime::currentTime());
    if(interval < 200 && interval > -200)
        return ;
    lasttime = QTime::currentTime();

    int x = row(item);

    switch (x) {
    case 0:
        lockWidgetClicked();
        break;
    case 1:
        switchWidgetClicked();
        break;
    case 2:
        logoutWidgetCliced();
        break;
    case 3:
        rebootWidgetClicked();
        break;
    case 4:
        shutdownWidgetClicked();
        break;
    default:
        break;
    }
}

void PowerManager::lockWidgetClicked()
{
    emit lock();
}

void PowerManager::switchWidgetClicked()
{
    emit switchToUser();
}

void PowerManager::shutdownWidgetClicked()
{
    QDBusInterface *interface = new QDBusInterface("org.gnome.SessionManager",
                                                   "/org/gnome/SessionManager",
                                                   "org.gnome.SessionManager",
                                                   QDBusConnection::sessionBus(),
                                                   this);

    QDBusMessage msg = interface->call("powerOff");
}

void PowerManager::rebootWidgetClicked()
{
    QDBusInterface *interface = new QDBusInterface("org.gnome.SessionManager",
                                                   "/org/gnome/SessionManager",
                                                   "org.gnome.SessionManager",
                                                   QDBusConnection::sessionBus(),
                                                   this);

    QDBusMessage msg = interface->call("reboot");
}

void PowerManager::logoutWidgetCliced()
{
    QDBusInterface *interface = new QDBusInterface("org.gnome.SessionManager",
                                                   "/org/gnome/SessionManager",
                                                   "org.gnome.SessionManager",
                                                   QDBusConnection::sessionBus(),
                                                   this);

    QDBusMessage msg = interface->call("logout");
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
    logoutFace->setPixmap(QPixmap(":/image/assets/logout.png").scaled(58,58));
    logoutLabel->setText(tr("Log Out"));
    logoutWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
    QVBoxLayout *logoutlayout = new QVBoxLayout(logoutWidget);
    logoutlayout->addWidget(logoutFace);
    logoutlayout->addWidget(logoutLabel);
    logoutWidget->installEventFilter(this);

    rebootWidget = new QWidget(this);
    rebootWidget->setObjectName("logoutWidget");
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
    insertItem(0, item0);
    setItemWidget(item0, lockWidget);

    QListWidgetItem *item1 = new QListWidgetItem();
    item1->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(1, item1);
    setItemWidget(item1, switchWidget);

    QListWidgetItem *item2 = new QListWidgetItem();
    item2->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(2, item2);
    setItemWidget(item2, logoutWidget);

    QListWidgetItem *item3 = new QListWidgetItem();
    item3->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(3, item3);
    setItemWidget(item3, rebootWidget);

    QListWidgetItem *item4 = new QListWidgetItem();
    item4->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(4, item4);
    setItemWidget(item4, shutdownWidget);

    adjustSize();

}
