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
#include "fullbackgroundwidget.h"

#include <QGuiApplication>
#include <QSvgRenderer>
#include <QScreen>
#include <QDBusInterface>
#include <QTime>
#include <QDebug>
#include <QPainter>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QCloseEvent>
#include <QDBusPendingReply>
#include <QImageReader>
#include <QX11Info>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/syslog.h>

#include <xcb/xcb.h>
#include "lockwidget.h"
#include "xeventmonitor.h"
#include "monitorwatcher.h"
#include "configuration.h"
#include "screensaver.h"
#include "screensaverwidget.h"
#include "grab-x11.h"
#include "tabletlockwidget.h"
// 实现键盘f1 - f2 的功能键
#include "PhysicalDeviceSet/sounddeviceset.h"
#include "PhysicalDeviceSet/brightnessdeviceset.h"

#include "config.h"

enum {
    SWITCH_TO_LINUX = 0,
    SWITCH_TO_ANDROID = 1,
    BACK_TO_DESKTOP = 2,
    TEST_CONNECTION = 3,
};

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

#define BLUR_RADIUS 300

QPixmap scaledPixmap(int width, int height, QString url)
{
    QFile imgFile(url);
    if(!imgFile.exists()){
        qDebug()<< "not find the pixmap file";
        return QPixmap();
    }
    QImage image(url);
    QPixmap pixmap = QPixmap::fromImage(image);
    if(pixmap.isNull()) {
        qDebug() << "pixmap is null";
        QProcess exec;
        QString program("file " + url);
        exec.start(program);
        exec.waitForFinished(1000);
        QString output = exec.readAllStandardOutput();
        qDebug() << output;
        if(output.contains("SVG")){
            qDebug() << "image format is SVG";
            QSvgRenderer render(url);
            QImage image(width, height, QImage::Format_ARGB32);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            render.render(&painter, image.rect());
            pixmap.convertFromImage(image);
        } else if(output.contains("TIFF")) {
            qDebug() << "image format is TIFF";

        }
    }

    return pixmap.scaled(width, height, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

int connect_to_switch(const char* path)
{
    int ret;
    int connect_fd;
    struct sockaddr_un srv_addr;

    connect_fd = socket(PF_UNIX,SOCK_STREAM,0);
    if(connect_fd < 0) {

        return -1;
    }

    srv_addr.sun_family=AF_UNIX;
    strcpy(srv_addr.sun_path, path);

    ret = connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret < 0) {
        perror("cannot connect to the server");
        close(connect_fd);
        return -1;
    }

    return connect_fd;
}

int switch_to_linux(const char* container)
{
    int connect_fd;
    int32_t switch_to = BACK_TO_DESKTOP;

    char path[1024] = {0};
    sprintf(path, "/var/lib/kydroid/%s/sockets/%s", container, "desktop-switch");
    printf("path = %s\n",path);
    connect_fd = connect_to_switch(path);

    if(connect_fd < 0)
        return -1;

    write(connect_fd, &switch_to, sizeof(switch_to));
    close(connect_fd);
    return 0;
}

void x11_get_screen_size(int *width,int *height)
{
    Display* display;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot connect to X server %s/n", "simey:0");
        exit (-1);
    }
    int screen_num;

    screen_num = DefaultScreen(display);

    *width = DisplayWidth(display, screen_num);
    *height = DisplayHeight(display, screen_num);
    XCloseDisplay(display);

}

QPixmap blurPixmap(QPixmap pixmap)
{
    QPainter painter(&pixmap);
    QImage srcImg = pixmap.toImage();
    qt_blurImage(&painter, srcImg, BLUR_RADIUS, false, false);
    painter.end();
    return pixmap;
}

FullBackgroundWidget::FullBackgroundWidget(QWidget *parent)
    : QWidget(parent),
      lockWidget(nullptr),
      xEventMonitor(new XEventMonitor(this)),
      monitorWatcher(new MonitorWatcher(this)),
      configuration(Configuration::instance()),
      isLocked(false),
      isPassed(false),
      lockState(false),
      screenStatus(UNDEFINED),
      isBlank(false),
      m_delay(false)
{
    qDebug() << "init - screenStatus: " << screenStatus;
    setMouseTracking(true);
    connect(monitorWatcher, &MonitorWatcher::monitorCountChanged,
            this, &FullBackgroundWidget::onScreenCountChanged);
    QDesktopWidget *desktop = QApplication::desktop();

    connect(desktop, &QDesktopWidget::resized,
            this, &FullBackgroundWidget::onDesktopResized);
    connect(desktop, &QDesktopWidget::workAreaResized,
            this, &FullBackgroundWidget::onDesktopResized);
     connect(desktop, &QDesktopWidget::primaryScreenChanged,
            this, &FullBackgroundWidget::onDesktopResized);
    connect(desktop, &QDesktopWidget::screenCountChanged,
            this, &FullBackgroundWidget::onDesktopResized);

    QDBusInterface *iface = new QDBusInterface("org.freedesktop.login1",
                                               "/org/freedesktop/login1",
                                               "org.freedesktop.login1.Manager",
                                               QDBusConnection::systemBus(),
                                               this);
    connect(iface, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));

#ifdef USE_INTEL
    QDBusConnection::systemBus().connect("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                           this, SLOT(propertiesChangedSlot(QString, QMap<QString, QVariant>, QStringList)));
#endif

    init();
    qApp->installNativeEventFilter(this);
    installEventFilter(this);
    QTimer::singleShot(500,this,SLOT(switchToLinux()));

    LogindIntegration *m_logind = new LogindIntegration(this);
    
    connect(m_logind, &LogindIntegration::requestUnlock, this,
        [this]() {
        closeScreensaver();
    });

}

#ifdef USE_INTEL
void FullBackgroundWidget::propertiesChangedSlot(QString property, QMap<QString, QVariant> propertyMap, QStringList propertyList)
{
    Q_UNUSED(property);
    Q_UNUSED(propertyList);
    if (propertyMap.keys().contains("LidIsClosed")) {
        qDebug() <<"LidIsClosed" <<  propertyMap.value("LidIsClosed").toBool();
//        if(!(propertyMap.value("LidIsClosed").toBool()))
//        {
//            m_isSuspend = true;
//        }
    }
}
#endif

void FullBackgroundWidget::switchToLinux()
{
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    char *username = pwd->pw_name;
    int uid = pwd->pw_uid;
    char container[100]= {0};

    sprintf(container,"kydroid2-%d-%s",uid,username);

    switch_to_linux(container);

}

void FullBackgroundWidget::laterActivate()
{
    activateWindow();
    raise();
    setFocus();
    if(lockWidget && lockWidget->isVisible())
        lockWidget->setFocus();
}

void FullBackgroundWidget::setLockState()
{
    if(lockState == true)
        return ;

    lockState = true;

    QDBusInterface *interface = new QDBusInterface(SS_DBUS_SERVICE,
                                                      SS_DBUS_PATH,
                                                      SS_DBUS_INTERFACE);
    QDBusMessage msg = interface->call("SetLockState");
    if(msg.type() == QDBusMessage::ErrorMessage)
           qDebug() << msg.errorMessage();
    
}

bool FullBackgroundWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::WindowDeactivate){
         QTimer::singleShot(50,this,SLOT(laterActivate()));
    }else if(event->type() == QEvent::WindowActivate){
        QTimer::singleShot(500,this,SLOT(setLockState()));
    }
    return false;
}

void FullBackgroundWidget::setIsStartup(bool val)
{
    isStartup = val;
}

void FullBackgroundWidget::paintEvent(QPaintEvent *event)
{
    for(auto screen : QGuiApplication::screens())
    {
        QPainter painter(this);
        if(background.isNull()){
            QColor cor = "#035290";
            painter.setBrush(cor);
            painter.drawRect(screen->geometry());
        }
    else{
            painter.drawPixmap(screen->geometry(), background);
        }
    }
    return QWidget::paintEvent(event);
}

void FullBackgroundWidget::closeEvent(QCloseEvent *event)
{
    qDebug() << "FullBackgroundWidget::closeEvent";

#ifdef USE_INTEL
    //蓝牙连接后 唤醒信号会有延迟 以防退出时未收到信号导致kwin compositor未resume
    QDBusInterface *interface = new QDBusInterface("org.ukui.KWin",
                                                   "/Compositor",
                                                   "org.ukui.kwin.Compositing",
                                                   QDBusConnection::sessionBus(),
                                                   this);

    if (!interface->isValid()){
        syslog(LOG_DEBUG, "interface error");
        return;
    }
    QDBusMessage msg = interface->call("resume");
    syslog(LOG_DEBUG, "after close resume kwin ");
#endif

    for(auto obj: children())
    {
        QWidget *widget = dynamic_cast<QWidget*>(obj);
        if(widget)
            widget->close();
    }
    closeGrab();

    if(isStartup){
        QProcess process;
        process.start("killall screensaver-focus-helper");
        process.waitForFinished(1000);
    }
    return QWidget::closeEvent(event);
}

void FullBackgroundWidget::showEvent(QShowEvent *event)
{
   // XSetWindowAttributes top_attrs;
   // top_attrs.override_redirect = False;
   // XChangeWindowAttributes(QX11Info::display(), this->winId(), CWOverrideRedirect, &top_attrs);
   // XRaiseWindow(QX11Info::display(), this->winId());
  //  raise();

    return QWidget::showEvent(event);
}


bool FullBackgroundWidget::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
        if (qstrcmp(eventType, "xcb_generic_event_t") != 0) {
            return false;
        }
        xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t*>(message);
        const uint8_t responseType = event->response_type & ~0x80;
        if (responseType == XCB_CONFIGURE_NOTIFY) {
            xcb_configure_notify_event_t *xc = reinterpret_cast<xcb_configure_notify_event_t*>(event);
            if(xc->window == winId())
                return false;
            XWindowAttributes window_attributes;
            XGetWindowAttributes (QX11Info::display(), xc->window,&window_attributes);
            XClassHint ch;
            ch.res_name = NULL;
            ch.res_class = NULL;
            XGetClassHint (QX11Info::display(), xc->window, &ch);
            if(QString(ch.res_name) == "ukui-screensaver-dialog")
                return false;

            laterActivate();
         }else if(responseType == XCB_MAP_NOTIFY){
            xcb_map_notify_event_t *xm = reinterpret_cast<xcb_map_notify_event_t*>(event);
            if(xm->window == winId())
                    return false;
            XWindowAttributes window_attributes;
            XGetWindowAttributes (QX11Info::display(), xm->window,&window_attributes);
            XClassHint ch;
            ch.res_name = NULL;
            ch.res_class = NULL;
            XGetClassHint (QX11Info::display(), xm->window, &ch);
            if(QString(ch.res_name) == "ukui-screensaver-dialog")
                return false;
            laterActivate();
	 }
        return false;
}

void FullBackgroundWidget::mouseMoveEvent(QMouseEvent *e)
{
	onCursorMoved(QCursor::pos()); 
	return QWidget::mouseMoveEvent(e);
}

void FullBackgroundWidget::mousePressEvent(QMouseEvent *e)
{
#ifdef USE_INTEL
    if(screenStatus == SCREEN_LOCK_AND_SAVER)
    {
        ScreenSaver *saver = configuration->getScreensaver();
        if(saver->path == "/usr/lib/ukui-screensaver/ukui-screensaver-default")
               return ;
        clearScreensavers();
    }
#endif
}

void FullBackgroundWidget::init()
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
                   | Qt::X11BypassWindowManagerHint);

    XWindowAttributes rootAttr;
    XGetWindowAttributes(QX11Info::display(), QX11Info::appRootWindow(), &rootAttr);
    XSelectInput( QX11Info::display(), QX11Info::appRootWindow(),
                  SubstructureNotifyMask|rootAttr.your_event_mask );
//    setAttribute(Qt::WA_DeleteOnClose);
/*捕获键盘，如果捕获失败，则可能是由于弹出菜单项已经捕获，那么模拟一次esc按键来退出菜单，如果仍捕获失败，则放弃锁屏，避免密码无法输入*/
    if(establishGrab())
        qDebug()<<"establishGrab : true";
    else {
        qDebug()<<"establishGrab : false";
        XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(),XK_Escape), True, 1);
        XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(),XK_Escape), False, 1);
        XFlush(QX11Info::display());
        sleep(1);
        if(!establishGrab())
        {
            exit(1);
        }
    }
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
    connect(xEventMonitor, SIGNAL(buttonPress(int, int)),
            this, SLOT(onGlobalButtonPressed(int, int)));


//    int totalWidth = 0;
//    int totalHeight = 0;
//    for(auto screen : QGuiApplication::screens())
//    {
//        totalWidth += screen->geometry().width();
//        totalHeight += screen->geometry().height();
//    }
//    setGeometry(0, 0, totalWidth, totalHeight);
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->geometry());
    
    QImageReader reader;
    reader.setFileName(configuration->getBackground());
    reader.setAutoTransform(true);
    reader.setScaledSize(QApplication::primaryScreen()->size());
    background = QPixmap::fromImageReader(&reader);
    
//    background.load(configuration->getBackground());
//    background = scaledPixmap(QApplication::primaryScreen()->geometry().width(), QApplication::primaryScreen()->geometry().height(), configuration->getBackground());
    if(!background.isNull()){
        background = blurPixmap(background);
    }
    
    xEventMonitor->start();

#ifdef USE_INTEL
    SoundDeviceSet::instance();
#endif
}

void FullBackgroundWidget::onCursorMoved(const QPoint &pos)
{
    if(!lockWidget)
    {
        return;
    }
    for(auto screen : QGuiApplication::screens())
    {
#ifdef USE_INTEL
        if(screen == qApp->primaryScreen()){
            lockWidget->setGeometry(screen->geometry());
            break;
        }
#else
        if(screen->geometry().contains(pos))
        {
            if(lockWidget->geometry() == screen->geometry())
                return ;
            /*避免切换时闪烁*/
            qDebug()<<screen->geometry()<<lockWidget->geometry();
            lockWidget->hide();
            lockWidget->setGeometry(screen->geometry());
            lockWidget->show();
            break;
        }
#endif
    }
}

void FullBackgroundWidget::lock()
{
    showLockWidget();
    if(lockWidget){
        lockWidget->show();
        lockWidget->startAuth();
    }
#ifndef USE_INTEL
    inhibit();
#endif
}


void FullBackgroundWidget::showLockWidget()
{
#ifdef USE_INTEL
    screenStatus = /*(ScreenStatus)(screenStatus | SCREEN_LOCK)*/SCREEN_LOCK;
#else
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
#endif
    qDebug() << "showLockWidget - screenStatus: " << screenStatus;

    if(!lockWidget)
    {
#ifdef USE_INTEL
        lockWidget = new TabletLockWidget(this);
        connect(lockWidget, &TabletLockWidget::closed,
                this, &FullBackgroundWidget::close);
        connect(lockWidget, &TabletLockWidget::screenSaver,
                this, [=] {
            showScreensaver();
        });
        connect(lockWidget, &TabletLockWidget::blackSaver,
                this, [=] {
            onShowBlackBackGround();
        });
#else
        lockWidget = new LockWidget(this);
        connect(lockWidget, &LockWidget::closed,
                this, &FullBackgroundWidget::close);
#endif
    }
    onCursorMoved(QCursor::pos());
    lockWidget->setFocus();
    XSetInputFocus(QX11Info::display(),this->winId(),RevertToParent,CurrentTime);
    activateWindow();
    repaint();
}

void FullBackgroundWidget::showScreensaver()
{
#ifdef USE_INTEL
    screenStatus = /*(ScreenStatus)(screenStatus | SCREEN_SAVER)*/SCREEN_LOCK_AND_SAVER;
#else
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_SAVER);
#endif
    qDebug() << "showScreensaver - screenStatus: " << screenStatus;

    for(auto screen : QGuiApplication::screens())
    {
        ScreenSaver *saver = configuration->getScreensaver();
        ScreenSaverWidget *saverWidget = new ScreenSaverWidget(saver, this);
        qDebug() << " new ScreenSaverWidget";
        widgetXScreensaverList.push_back(saverWidget);
        //深色模式有一像素的白边，所以主屏幕向左，向右移一个像素点;这种操作后，外显上方仍旧会有一个像素的白边，暂时不对外显做偏移处理
        if(screen == qApp->primaryScreen()) {
            saverWidget->setGeometry(screen->geometry().x()-1, screen->geometry().y()-1,
                                     screen->geometry().width()+1, screen->geometry().height()+1);
        } else {
            saverWidget->setGeometry(screen->geometry());
        }

    }
    setCursor(Qt::BlankCursor);

    //显示屏保时，停止认证（主要针对生物识别）
    if(lockWidget)
    {
        lockWidget->stopAuth();
	lockWidget->hide();
    }
}

void FullBackgroundWidget::clearScreensavers()
{
#ifdef USE_INTEL
    screenStatus = /*(ScreenStatus)(screenStatus & ~SCREEN_SAVER)*/SCREEN_LOCK;
#else
    screenStatus = (ScreenStatus)(screenStatus & ~SCREEN_SAVER);
#endif
    for(auto widget : widgetXScreensaverList)
    {
        widget->close();
    }
    widgetXScreensaverList.clear();

    qDebug() << "clearScreensavers - screenStatus: " << screenStatus;

    unsetCursor();
    if(screenStatus == UNDEFINED)
    {
        close();
    }
    else
    {
        lock();
    }
}

int FullBackgroundWidget::onSessionStatusChanged(uint status)
{
    qDebug() << "session status changed: " << status;
    if(status != SESSION_IDLE)
    {
        //当前session没有处于空闲状态
        return -1;
    }
    qDebug() << "onSessionStatusChanged - screenStatus: " << screenStatus;

    if(!configuration->xscreensaverActivatedWhenIdle())
    {
        return -1;
    }
#ifdef USE_INTEL
    if(screenStatus == SCREEN_LOCK_AND_SAVER)
#else
    if(screenStatus & SCREEN_SAVER)
#endif
    {
        return -1;
    }
#ifdef USE_INTEL
    else if(screenStatus == SCREEN_LOCK)
#else
    else if(screenStatus & SCREEN_LOCK)
#endif
    {
        showScreensaver();
    }
    else if(screenStatus == UNDEFINED)
    {
#ifdef USE_INTEL
        //显示锁屏和屏保
        showLockWidget();
        showScreensaver();
#else
        if(configuration->xscreensaverActivatedWhenIdle() &&
                configuration->lockWhenXScreensaverActivated())
        {
            //显示锁屏和屏保
            showLockWidget();
            showScreensaver();
        }
        else if(configuration->xscreensaverActivatedWhenIdle())
        {
            //只显示屏保
            showScreensaver();
        }
#endif
    }
    return 0;
}

void FullBackgroundWidget::onBlankScreensaver()
{
      showLockWidget();
      screenStatus = (ScreenStatus)(screenStatus | SCREEN_SAVER | SCREEN_LOCK);
      qDebug() << "showScreensaver - screenStatus: " << screenStatus;

      for(auto screen : QGuiApplication::screens())
      {
          ScreenSaver *saver = configuration->getScreensaver();
          saver->mode = SaverMode(SAVER_BLANK_ONLY);
          ScreenSaverWidget *saverWidget = new ScreenSaverWidget(saver, this);
          widgetXScreensaverList.push_back(saverWidget);
          saverWidget->setGeometry(screen->geometry());
      }
      setCursor(Qt::BlankCursor);
      isBlank = true;
}

void FullBackgroundWidget::onScreensaver()
{
      showLockWidget();
      showScreensaver();
}

void FullBackgroundWidget::onGlobalKeyPress(const QString &key)
{
#ifdef USE_INTEL
    qDebug() << "onGlobalKeyPress " << key << "screenStatus " << screenStatus;

    if(m_delay)
    {
        qDebug() << "it is delay time ,ignore";
        return;
    }

    if(!key.isEmpty() && (screenStatus == SCREEN_LOCK_AND_SAVER))
    {
        clearScreensavers();
    }/*else{
       lockWidget->startAuth();
        inhibit();
    }*/
//    if(screenStatus == SCREEN_LOCK)
//    {
//        checkNumLock();
//        int keyValue = numberMatch(key);;
//        if (keyValue >= 0 && keyValue <= 10 )
//        {
//            lockWidget->RecieveKey(keyValue);
//        }
//    }
#endif
}

void FullBackgroundWidget::onGlobalKeyRelease(const QString &key)
{
    if(key == "Caps_Lock")
    {
        lockWidget->capsLockChanged();
    }
#ifdef USE_INTEL

    // 声音、亮度等调整
    // 取消声音快捷键设置，由settings-deamon控制   有一个bug，音量为零时再按静音键，静音指示灯会熄灭，可解决，但是不用锁屏做了
//    if(key == "XF86AudioRaiseVolume")
//    {
//        SoundDeviceSet::instance()->setValue(SoundDeviceSet::instance()->getValue() + 5);
//    }
//    else if(key == "XF86AudioLowerVolume")
//    {
//        SoundDeviceSet::instance()->setValue(SoundDeviceSet::instance()->getValue() - 5);
//    }
//    else if(key == "XF86AudioMute")
//    {
//        SoundDeviceSet::instance()->setMute(!(SoundDeviceSet::instance()->getIsMute()));
//    }
//    else

    if (key == "XF86MonBrightnessUp") // 亮度调整
    {
        //qDebug() << "up";
        BrightnessDeviceSet* pBrightness = BrightnessDeviceSet::instance();
        pBrightness->setValue(pBrightness->getValue() + 5);
    }
    else if (key == "XF86MonBrightnessDown")
    {
        //qDebug() << "down";
        BrightnessDeviceSet* pBrightness = BrightnessDeviceSet::instance();
        pBrightness->setValue(pBrightness->getValue() - 5);
    }
    else if (key == "XF86RFKill") // 飞行模式
    {
        // 键盘上的飞行模式实体键 生效，不需要在登录界面进行设置
    }
    else if (key == "") // num_lock
    {
        // 键盘上的num_lock生效、不需要登录界面进行管理
    }
#else
    if(key == "Escape" && screenStatus == SCREEN_LOCK)
    {
	showScreensaver();
    }
    else if(screenStatus & SCREEN_SAVER && !isBlank)
    {
        clearScreensavers();	
    }
#endif
}

void FullBackgroundWidget::onGlobalButtonDrag(int xPos, int yPos)
{
#ifdef USE_INTEL
    if(screenStatus == SCREEN_LOCK_AND_SAVER)
    {
        ScreenSaver *saver = configuration->getScreensaver();
        if(saver->path == "/usr/lib/ukui-screensaver/ukui-screensaver-default")
               return ;
        clearScreensavers();
    }
#else
    if(screenStatus & SCREEN_SAVER && !isBlank)
    {
        ScreenSaver *saver = configuration->getScreensaver();
        if(isPassed || saver->path != "/usr/lib/ukui-screensaver/ukui-screensaver-default"){
            clearScreensavers();
        }
        isPassed = true;
    }
#endif
}

void FullBackgroundWidget::onGlobalButtonPressed(int xPos, int yPos)
{
#ifndef USE_INTEL
    if(screenStatus & SCREEN_SAVER)
    {
        clearScreensavers();
    }
#endif
}

void FullBackgroundWidget::closeScreensaver()
{
    if(screenStatus & SCREEN_SAVER){
    	clearScreensavers();
    }

    if(screenStatus & SCREEN_LOCK){
    	if(lockWidget){
    	    	lockWidget->stopAuth();
		lockWidget->close();
    	}
    }

    close();
}

void FullBackgroundWidget::onScreenCountChanged(int)
{
#ifdef USE_INTEL
    QSize newSize = monitorWatcher->getVirtualSize();
    setGeometry(0, 0, newSize.width(), newSize.height());
    //repaint();
    update();
    clearScreensavers();
#else
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->geometry());
    //repaint();
    if(screenStatus & SCREEN_SAVER)
    {
        clearScreensavers();
    }
    update();
#endif
}


void FullBackgroundWidget::onDesktopResized()
{
#ifdef USE_INTEL
    qDebug() << "[FullBackgroundWidget] [onDesktopResized]";
    QDesktopWidget *desktop = QApplication::desktop();
    if(NULL == desktop)
    {
        qWarning() << " get desktop size failed";
        return;
    }
    setGeometry(desktop->geometry());
    if(lockWidget)
        onCursorMoved(cursor().pos());
//    clearScreensavers();
   //repaint();
    update();
    if(screenStatus == SCREEN_LOCK_AND_SAVER)
    {
        clearScreensavers();
        showScreensaver();
    }
#else
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->geometry());
    if(lockWidget)
    	onCursorMoved(QCursor::pos());
    if(screenStatus & SCREEN_SAVER)
    {
        clearScreensavers();
    }
    //repaint();
    update();
#endif
}

void FullBackgroundWidget::laterInhibit(bool val)
{
	if(val){
		inhibit();
	}else{
		uninhibit();
	}
}

void FullBackgroundWidget::laterStartAuth()
{
	lockWidget->startAuth();
	inhibit();
}

void FullBackgroundWidget::onPrepareForSleep(bool sleep)
{
    ///系统休眠时，会关闭总线，导致设备不可用，发生错误
    ///在系统休眠之前停止认证，在系统唤醒后重新开始认证
    if(sleep)
    {
        lockWidget->stopAuth();
        uninhibit();
    }
    else
    {
        if(screenStatus & SCREEN_SAVER)
        {
	    isBlank = false;
            clearScreensavers();
        }else{
            lockWidget->startAuth();
            inhibit();
        }
    }
}

void FullBackgroundWidget::inhibit()
{
    if (m_inhibitFileDescriptor.isValid()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                                          "/org/freedesktop/login1",
                                                          "org.freedesktop.login1.Manager",
                                                          QStringLiteral("Inhibit"));
    message.setArguments(QVariantList({QStringLiteral("sleep"),
                                       "Screen Locker",
                                       "Ensuring that the screen gets locked before going to sleep",
                                       QStringLiteral("delay")}));
    QDBusPendingReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(message);
    if (!reply.isValid()) {
        return;
    }
    reply.value().swap(m_inhibitFileDescriptor);
}

void FullBackgroundWidget::uninhibit()
{
    if (!m_inhibitFileDescriptor.isValid()) {
        return;
    }
    m_inhibitFileDescriptor = QDBusUnixFileDescriptor();
}

/**
 * @brief FullBackgroundWidget::getPaddingPixmap
 * @param pixmap 需要填充的图像
 * @param width  容器宽度
 * @param height 容器高度
 * @return
 */
QPixmap FullBackgroundWidget::getPaddingPixmap(QPixmap pixmap, int width, int height)
{
    if (pixmap.isNull() || pixmap.width() == 0 || pixmap.height() == 0)
    {
        return QPixmap();
    }

    bool useHeight;
    float scaled = 0.0;
    QPixmap scaledPixmap;
    QPixmap paddingPixmap;
    qint64 rw = qint64(height) * qint64(pixmap.width()) / qint64(pixmap.height());

    useHeight = (rw >= width);
    if (useHeight) {
        scaled = float(height) / float(pixmap.height());
        scaledPixmap = pixmap.scaled(pixmap.width() * scaled, height);
        paddingPixmap = scaledPixmap.copy((pixmap.width() * scaled - width) / 2 , 0, width, height);
    } else {
        scaled = float(width) / float(pixmap.width());
        scaledPixmap = pixmap.scaled(width, pixmap.height() * scaled);
        paddingPixmap = scaledPixmap.copy(0 , (pixmap.height() * scaled - height) / 2,width, height);
    }

    return paddingPixmap;
}

#ifdef USE_INTEL
void FullBackgroundWidget::onShowBlackBackGround()
{
    screenStatus = SCREEN_BLACK;
    qDebug() << "showBlackBackGround - screenStatus: " << screenStatus;

    for(auto screen : QGuiApplication::screens())
    {
//        BlackWidget *blackWidget = new BlackWidget(this);
//        qDebug() << " new BlackWidget";
//        widgetBlackList.push_back(blackWidget);
        ScreenSaver *saver = configuration->getScreensaver();
      saver->mode = SaverMode(SAVER_BLANK_ONLY);
      ScreenSaverWidget *saverWidget = new ScreenSaverWidget(saver, this);
      widgetBlackList.push_back(saverWidget);
      saverWidget->setGeometry(screen->geometry());

        //深色模式有一像素的白边，所以主屏幕向左，向右移一个像素点;这种操作后，外显上方仍旧会有一个像素的白边，暂时不对外显做偏移处理
        if(screen == qApp->primaryScreen()) {
            saverWidget->setGeometry(screen->geometry().x()-1, screen->geometry().y()-1,
                                     screen->geometry().width()+1, screen->geometry().height()+1);
        } else {
            saverWidget->setGeometry(screen->geometry());
        }


    }
    setCursor(Qt::BlankCursor);
}
#endif
