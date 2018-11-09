#include "fullbackgroundwidget.h"

#include <QGuiApplication>
#include <QScreen>
#include <QDBusInterface>
#include <QDebug>
#include <QPainter>

#include "lockwidget.h"
#include "xeventmonitor.h"
#include "monitorwatcher.h"
#include "configuration.h"
#include "screensaver.h"
#include "screensaverwidget.h"

FullBackgroundWidget::FullBackgroundWidget(QWidget *parent)
    : QWidget(parent),
      lockWidget(nullptr),
      xEventMonitor(new XEventMonitor(this)),
      monitorWatcher(new MonitorWatcher(this)),
      configuration(new Configuration(this)),
      isLocked(false),
      screenStatus(UNDEFINED)
{
    qDebug() << "init " << screenStatus;
    init();
    onSessionStatusChanged(SESSION_IDLE);
}

void FullBackgroundWidget::paintEvent(QPaintEvent *event)
{
    for(auto screen : QGuiApplication::screens())
    {
        QPainter painter(this);
        painter.drawPixmap(screen->geometry(), background);
    }
    return QWidget::paintEvent(event);
}


void FullBackgroundWidget::init()
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
                   | Qt::X11BypassWindowManagerHint);
//    setAttribute(Qt::WA_DeleteOnClose);

    // 监听session信号
    smInterface = new QDBusInterface(SM_DBUS_SERVICE,
                                     SM_DBUS_PATH,
                                     SM_DBUS_INTERFACE,
                                     QDBusConnection::sessionBus());
    connect(smInterface, SIGNAL(StatusChanged(uint)),
            this, SLOT(onSessionStatusChanged(uint)));

    connect(xEventMonitor, SIGNAL(keyPress(const QString &)),
            this, SLOT(onGlobalKeyPress(const QString &)));
    connect(xEventMonitor, SIGNAL(keyRelease(const QString &)),
            this, SLOT(onGlobalKeyRelease(const QString &)));
    connect(xEventMonitor, SIGNAL(buttonDrag(int, int)),
            this, SLOT(onGlobalButtonDrag(int, int)));

    int totalWidth = 0;
    int totalHeight = 0;
    for(auto screen : QGuiApplication::screens())
    {
        totalWidth += screen->geometry().width();
        totalHeight += screen->geometry().height();
    }
    setGeometry(0, 0, totalWidth, totalHeight);

    background.load(configuration->getBackground());

    xEventMonitor->start();
}

void FullBackgroundWidget::onCursorMoved(const QPoint &pos)
{
    if(!lockWidget)
    {
        return;
    }
    for(auto screen : QGuiApplication::screens())
    {
        if(screen->geometry().contains(pos))
        {
            lockWidget->setGeometry(screen->geometry());
            break;
        }
    }
}


void FullBackgroundWidget::lock()
{
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    qDebug() << "lock " << screenStatus;
    isLocked = true;
    lockWidget = new LockWidget(this);
    connect(lockWidget, &LockWidget::closed,
            this, &FullBackgroundWidget::close);
    onCursorMoved(cursor().pos());

}

void FullBackgroundWidget::showScreensaver()
{
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_SAVER);
    qDebug() << "showsaver " << screenStatus;
    for(auto screen : QGuiApplication::screens())
    {
        ScreenSaver *saver = configuration->getScreensaver();
        qDebug() << *saver;
        ScreenSaverWidget *saverWidget = new ScreenSaverWidget(saver, this);
        widgetXScreensaverList.push_back(saverWidget);
        saverWidget->setGeometry(screen->geometry());
    }
    setCursor(Qt::BlankCursor);
    if(lockWidget)
    {
        lockWidget->stopAuth();
    }
}

void FullBackgroundWidget::clearScreensavers()
{
    for(auto widget : widgetXScreensaverList)
    {
        widget->close();
    }
    widgetXScreensaverList.clear();
    screenStatus = (ScreenStatus)(screenStatus & ~SCREEN_SAVER);
    qDebug() << "clear saver " << screenStatus;
    lockWidget->setFocus();

    unsetCursor();
//    if(screenStatus == UNDEFINED)
//    {
//        close();
//    }
    if(lockWidget)
    {
        lockWidget->startAuth();
    }
}

void FullBackgroundWidget::onSessionStatusChanged(uint status)
{
    qDebug() << "session status changed: " << status;
    if(status != SESSION_IDLE)
    {
        //当前session没有处于空闲状态
        return;
    }

    if(screenStatus == SCREEN_LOCK)
    {
        //当前处于锁屏界面
        showScreensaver();
        return;
    }

    if(configuration->xscreensaverActivatedWhenIdle() &&
            configuration->lockWhenXScreensaverActivated())
    {
        //显示锁屏和屏保
        lock();
        showScreensaver();
    }
    else if(configuration->xscreensaverActivatedWhenIdle())
    {
        //只显示屏保
        showScreensaver();
    }
    else
    {

    }
}

void FullBackgroundWidget::onGlobalKeyPress(const QString &key)
{
//    qDebug() << key << "pressed";

}

void FullBackgroundWidget::onGlobalKeyRelease(const QString &key)
{
    if(key == "Caps_Lock")
    {
        lockWidget->capsLockChanged();
    }
    else if(key == "Escape" && screenStatus == SCREEN_LOCK)
    {
        showScreensaver();
    }
    else if(screenStatus & SCREEN_SAVER)
    {
        clearScreensavers();
    }
}

void FullBackgroundWidget::onGlobalButtonDrag(int xPos, int yPos)
{
//    qDebug() << "button move to " << xPos << yPos;
    if(screenStatus & SCREEN_SAVER)
    {
        clearScreensavers();
    }
}
