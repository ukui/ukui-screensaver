#ifndef SCREENSAVERWIDGET_H
#define SCREENSAVERWIDGET_H

#include <QWidget>
#include "screensaver.h"


class ScreenSaverWidget : public QWidget
{
    Q_OBJECT
public:
    ScreenSaverWidget(ScreenSaver *screensaver,
                      QWidget *parent = nullptr);
    ~ScreenSaverWidget() {qDebug() << "~ScreenSaverWidget";}

protected:
    void closeEvent(QCloseEvent *);
    void paintEvent(QPaintEvent *event);

private:
    void embedXScreensaver(const QString &path);

private Q_SLOTS:
    void onBackgroundChanged(const QString &path);

private:
    QTimer          *timer;
    int             xscreensaverPid;
    ScreenSaver     *screensaver;
    bool            closing;
    float           opacity;
};

#endif // SCREENSAVERWIDGET_H
