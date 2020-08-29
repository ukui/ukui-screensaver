#include <QDateTime>
#include <QTimer>
#include <QDebug>
#include <QMenu>
//#include <QtX11Extras/QX11Info>
//#include <X11/Xlib.h>
//#include <X11/XKBlib.h>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFont>
#include <QPalette>

#include "tabletlockwidget.h"
#include "authdialog.h"
#include "virtualkeyboard.h"
#include "users.h"
#include "displaymanager.h"
#include "weathermanager.h"

float tablescale;
TabletLockWidget::TabletLockWidget(QWidget *parent) :
    QWidget(parent),
    usersMenu(nullptr),
    users(new Users(this)),
    displayManager(new DisplayManager(this)),
    weatherManager(new WeatherManager(this))
{
    tablescale = 1.0;
    this->installEventFilter(this);

    connect(weatherManager, &WeatherManager::onWeatherUpdate,
            this, &TabletLockWidget::getWeatherFinish);
    initUI();
}

TabletLockWidget::~TabletLockWidget()
{
}

void TabletLockWidget::closeEvent(QCloseEvent *event)
{
    qDebug() << "LockWidget::closeEvent";
    return QWidget::closeEvent(event);
}

bool TabletLockWidget::eventFilter(QObject *obj, QEvent *event)
{
//    if(event->type() == 2){
//        qDebug() << "---------event->type() == 2";
//    }

    return false;
}

void TabletLockWidget::startAuth()
{

}

void TabletLockWidget::stopAuth()
{

}

void TabletLockWidget::initUI()
{
    //    qDebug() << "init ui"

    // 使用代码方式重写布局
    m_widget_center = new QWidget(this);
    m_widget_center->setContentsMargins(0,0,0,0);

    m_layout_center = new QVBoxLayout(m_widget_center);

    //    m_layout_weather = new QHBoxLayout(m_layout_center);

    //天气模块
    m_widget_weather = new QWidget(this);
    m_widget_weather->setStyleSheet("QWidget#widgetMain{border: 1px solid #FFFFFF; border-radius: 5px;}");

    m_layout_weather = new QHBoxLayout(m_widget_weather);
    m_layout_weather->setSpacing(0);
    m_layout_weather->setMargin(0);
    QSpacerItem *spacer = new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_layout_weather->addItem(spacer);

    m_label_weather_icon = new QLabel(m_widget_weather);
    m_layout_weather->addWidget(m_label_weather_icon);
    m_label_weather_icon->setPixmap(weatherManager->getWeatherIcon());

    QSpacerItem *spacerWeatherIcon2Location = new QSpacerItem(8, 20, QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_layout_weather->addItem(spacerWeatherIcon2Location);
    //    m_layout_weather->addSpacing(8);

    m_label_weather_location_category = new QLabel(m_widget_weather);
    m_label_weather_location_category->setStyleSheet("QLabel{color:white; font-size: 26px;}");
    m_label_weather_location_category->setText("");//呼和浩特市郊区·中到大雨
    m_label_weather_location_category->setMaximumSize(QSize(16777215, 16777215));
    m_layout_weather->addWidget(m_label_weather_location_category);

    QSpacerItem *spacerWeatherLocation2Tem = new QSpacerItem(8, 20, QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_layout_weather->addItem(spacerWeatherLocation2Tem);
    //    m_layout_weather->addSpacing(8);

    m_label_weather_temperatrue = new QLabel(m_widget_weather);
    m_label_weather_temperatrue->setStyleSheet("QLabel{color:white; font-size: 26px;}");
    m_layout_weather->addWidget(m_label_weather_temperatrue);

    QSpacerItem *spacer2 = new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_layout_weather->addItem(spacer2);

    weatherManager->getWeather();

    m_layout_center->addWidget(m_widget_weather);

    m_label_time = new QLabel(m_widget_center);
    m_layout_center->addWidget(m_label_time);

    m_label_date = new QLabel(m_widget_center);
    m_layout_center->addWidget(m_label_date);

    //显示系统时间
    timer = new QTimer(this);
    QString time = QDateTime::currentDateTime().toString("hh:mm");
    QString date = QDate::currentDate().toString("dddd yyyy/MM/dd");

    connect(timer, &QTimer::timeout, this, [&]{
        QString time = QDateTime::currentDateTime().toString("hh:mm");
        m_label_time->setText(time);
        QString date = QDate::currentDate().toString("dddd yyyy/MM/dd");
        m_label_date->setText(date);
    });

    m_label_time->setText(time);
    m_label_time->setStyleSheet("QLabel{color:white; font-size: 96px;}");
    m_label_time->setAlignment(Qt::AlignCenter);

    m_label_date->setText(date);
    m_label_date->setStyleSheet("QLabel{color:white; font-size: 24px;}");
    m_label_date->setAlignment(Qt::AlignCenter);


    //设置中间模块的位置
    m_widget_center->move(width()/2 - m_widget_center->width()/2, 95);

    //底部布局
    m_widget_buttom = new QWidget(this);
    m_widget_buttom->setContentsMargins(0, 0, 0, 0);
    m_widget_buttom->setFixedWidth(width());
    m_layout_buttom = new QGridLayout(m_widget_buttom);
    m_layout_buttom->setContentsMargins(36,0,36,0);

    //消息通知布局
    m_widget_notice = new QWidget(m_widget_buttom);
    m_label_notice_icon = new QLabel(m_widget_notice);
    m_label_notice_message = new QLabel(m_widget_notice);
    m_layout_notice = new QHBoxLayout(m_widget_notice);

    m_label_notice_icon->setFixedSize(24,24);
    m_label_notice_icon->setPixmap(QPixmap(":/image/assets/message.png"));
    m_layout_notice->addWidget(m_label_notice_icon);

    QSpacerItem *spacerNoticeIcon2Msg = new QSpacerItem(8, 20, QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_layout_notice->addSpacerItem(spacerNoticeIcon2Msg);
    m_label_notice_message->setText(tr("You have %1 unread message").arg("99+"));//消息要和侧边栏对接
    m_label_notice_message->setStyleSheet("QLabel{color:white; font-size: 16px;}");

    m_layout_notice->addWidget(m_label_notice_message);

    QSpacerItem *spacerNoticeMsg2border = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
    m_layout_notice->addSpacerItem(spacerNoticeMsg2border);

    QWidget *unlockMsgWidget = new QWidget(m_widget_buttom);
    unlockMsgWidget->setContentsMargins(0,0,0,0);

    QHBoxLayout *unlockLayout = new QHBoxLayout(unlockMsgWidget);
    unlockLayout->setContentsMargins(0,0,0,0);

    unlockLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Maximum));
    m_label_unlock_guide = new QLabel(unlockMsgWidget);
    m_label_unlock_guide->setText(tr("Slide to unlock"));
    m_label_unlock_guide->setAlignment(Qt::AlignCenter);
    m_label_unlock_guide->setStyleSheet("QLabel{color:white; font-size: 16px;}");
    unlockLayout->addWidget(m_label_unlock_guide);
    unlockLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Maximum));

    QWidget *unlockPbWidget = new QWidget(m_widget_buttom);
    unlockPbWidget->setContentsMargins(0,0,0,0);
    QHBoxLayout *unlockPbLayout = new QHBoxLayout(unlockPbWidget);
    unlockPbLayout->setContentsMargins(0,0,0,4);
    unlockPbLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Maximum));
    m_pb_up_unlock = new QPushButton(unlockPbWidget);
    m_pb_up_unlock->setFixedSize(32,32);
    m_pb_up_unlock->setIcon(QIcon(":/image/assets/slide_unlock.png"));
    unlockPbLayout->addWidget(m_pb_up_unlock);
    unlockPbLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Maximum));
    connect(m_pb_up_unlock, &QPushButton::clicked,
            this, &TabletLockWidget::closed);

    QWidget *settingPbWidget = new QWidget(m_widget_buttom);
    settingPbWidget->setContentsMargins(0,0,0,0);
    QHBoxLayout *settingPbLayout = new QHBoxLayout(settingPbWidget);
    settingPbLayout->setContentsMargins(0,0,0,0);

    settingPbLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Maximum));
    m_pb_login = new QPushButton(m_widget_buttom);
    m_pb_login->setIcon(QIcon(":/image/assets/more.png"));
    settingPbLayout->addWidget(m_pb_login);

    m_layout_buttom->addWidget(m_widget_notice, 0, 0);
    m_layout_buttom->addWidget(unlockMsgWidget, 0, 1);
    m_layout_buttom->addWidget(settingPbWidget, 0, 2);
    m_layout_buttom->addWidget(unlockPbWidget,  1, 1);
//    m_layout_buttom->setAlignment(Qt::AlignCenter);
}

void TabletLockWidget::showVirtualKeyboard()
{

}

void TabletLockWidget::showPowerManager()
{

}

void TabletLockWidget::switchToGreeter()
{

}

void TabletLockWidget::setVirkeyboardPos()
{

}

void TabletLockWidget::initUserMenu()
{
    //nothing
}

/* lockscreen follows cursor */
void TabletLockWidget::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    //重新计算缩放比例
    tablescale = QString::number(size.width() / 1920.0, 'f', 1).toFloat();

    if(tablescale > 1)
        tablescale = 1;

//    XSetInputFocus(QX11Info::display(),this->winId(),RevertToParent,CurrentTime);
}

void TabletLockWidget::onUserAdded(const UserItem &user)
{

}

void TabletLockWidget::onUserDeleted(const UserItem &user)
{

}

void TabletLockWidget::onUserMenuTrigged(QAction *action)
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

void TabletLockWidget::getWeatherFinish(QString city, QString cond, QString tmp)
{
    qDebug() << "getWeatherFinish";
    qDebug() << city << "," << cond << "," << tmp;

    m_label_weather_location_category->setText(city + "·" + cond);
    m_label_weather_temperatrue->setText(tmp + "°");
    m_label_weather_location_category->adjustSize();

    m_label_weather_icon->setPixmap(weatherManager->getWeatherIcon());
}

//重写重绘事件，比如动态更新文本位置等
void TabletLockWidget::paintEvent(QPaintEvent *event)
{
    m_widget_center->adjustSize();
    m_widget_buttom->adjustSize();
    m_widget_center->move(width()/2 - m_widget_center->width()/2, 95);

    m_widget_buttom->setFixedWidth(width());
    m_widget_buttom->move(0,height()-m_widget_buttom->height());
    //测试，显示控件边框
//    m_widget_buttom->setStyleSheet(QString::fromUtf8("border:1px solid green"));
//    m_widget_center->setStyleSheet(QString::fromUtf8("border:1px solid red"));
}
