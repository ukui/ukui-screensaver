#include "screensaverwidget.h"
#include <unistd.h>
#include <signal.h>
#include <QDebug>
#include <QTimer>
#include <QPainter>

ScreenSaverWidget::ScreenSaverWidget(ScreenSaver *screensaver, QWidget *parent)
    : QWidget(parent),
      timer(nullptr),
      xscreensaverPid(-1),
      screensaver(screensaver),
      closing(false)
{
    qDebug() << "ScreenSaverWidget::ScreenSaverWidget";
    qDebug() << *screensaver;
    setMouseTracking(true);
    setFocus();

    switch(screensaver->mode) {
    case SAVER_RANDOM:
    case SAVER_SINGLE:
        embedXScreensaver(screensaver->path);
        break;
    case SAVER_BLANK_ONLY:
    {
        QPalette plt;
        plt.setBrush(QPalette::Window, Qt::black);
        setPalette(plt);
        setAutoFillBackground(true);
        break;
    }
    case SAVER_IMAGE:
    {
        setAutoFillBackground(true);
        screensaver->startSwitchImages();

        QPalette plt;
        plt.setBrush(QPalette::Background, Qt::transparent);
        setPalette(plt);

        connect(screensaver, &ScreenSaver::imagePathChanged,
                this, &ScreenSaverWidget::onBackgroundChanged);
        break;
    }
    }
    show();
}

void ScreenSaverWidget::closeEvent(QCloseEvent */*event*/)
{
    qDebug() << "ScreenSaverWidget::closeEvent";
    if(xscreensaverPid > 0)
        kill(xscreensaverPid, SIGKILL);

    if(!closing){
        closing = true;
        screensaver->stopSwitchImages();
        delete screensaver;
        if(timer && timer->isActive())
            timer->stop();
        releaseKeyboard();
        releaseMouse();
        Q_EMIT closed();
    }
}

void ScreenSaverWidget::paintEvent(QPaintEvent *event)
{
    if(screensaver->mode == SAVER_IMAGE) {
        switch(screensaver->effect) {
        case TRANSITION_NONE:
        {
            QPixmap pixmap(screensaver->path);
            pixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QPainter painter(this);
            painter.drawPixmap(geometry(), pixmap);
            break;
        }
        case TRANSITION_FADE_IN_OUT:
        {
            QPainter painter(this);
            QPixmap pixmap1(screensaver->lastPath);
            pixmap1.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            painter.setOpacity(opacity);
            painter.drawPixmap(geometry(), pixmap1);

            QPixmap pixmap(screensaver->path);
            pixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            painter.setOpacity(1 - opacity);
            painter.drawPixmap(geometry(), pixmap);
            break;
        }
        }


    }
    return QWidget::paintEvent(event);
}


/* Embed xscreensavers */
void ScreenSaverWidget::embedXScreensaver(const QString &path)
{
    unsigned long wid = winId();
    char widStr[20] = {0};
    snprintf(widStr, sizeof(widStr), "%lu", wid);
    if((xscreensaverPid = fork()) == 0) {
        execl(path.toStdString().c_str(), "xscreensaver", "-window-id", widStr, (char*)0);
        qWarning() << "exec " << path << "failed";
    }
    qDebug() << "xscreensaver pid: " << xscreensaverPid;
}



void ScreenSaverWidget::onBackgroundChanged(const QString &/*path*/)
{
    switch(screensaver->effect) {
    case TRANSITION_NONE:
        repaint();
        break;
    case TRANSITION_FADE_IN_OUT:
        opacity = 1.0;
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [&]{
            opacity -= 0.1;
            if(opacity <= 0)
                timer->stop();
            else
                repaint();

        });
        timer->start(50);
        break;
    }
}

void ScreenSaverWidget::keyPressEvent(QKeyEvent */*event*/)
{
    close();
}

void ScreenSaverWidget::mouseMoveEvent(QMouseEvent */*event*/)
{
    qDebug() << "ScreenSaverWidget::mouseMoveEvent";
    close();
}
