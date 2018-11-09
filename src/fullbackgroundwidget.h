#ifndef FULLBACKGROUNDWIDGET_H
#define FULLBACKGROUNDWIDGET_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QWidget>
#include "types.h"

class LockWidget;
class XEventMonitor;
class MonitorWatcher;
class Configuration;
class QDBusInterface;

class FullBackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FullBackgroundWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);

public Q_SLOTS:
    void onCursorMoved(const QPoint &pos);
    void lock();
    void showScreensaver();

private:
    void init();
    void clearScreensavers();

private Q_SLOTS:
    void onSessionStatusChanged(uint status);
    void onGlobalKeyPress(const QString &key);
    void onGlobalKeyRelease(const QString &key);
    void onGlobalButtonDrag(int xPos, int yPos);

private:
    QDBusInterface      *smInterface;
    LockWidget          *lockWidget;
    XEventMonitor       *xEventMonitor;
    MonitorWatcher      *monitorWatcher;
    Configuration       *configuration;
    QList<QWidget*>     widgetXScreensaverList;
    QList<pid_t>        xscreensaverPidList;
    bool                isLocked;
    ScreenStatus        screenStatus;
    QPixmap             background;
};

#endif // FULLBACKGROUNDWIDGET_H
