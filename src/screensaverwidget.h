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

protected:
    void closeEvent(QCloseEvent *);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    void embedXScreensaver(const QString &path);

Q_SIGNALS:
    void closed();

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
