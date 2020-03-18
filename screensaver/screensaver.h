#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QGSettings>
#include <QString>
#include "sleeptime.h"
#include "chinesedate.h"

class Screensaver : public QWidget
{
    Q_OBJECT

public:
    explicit Screensaver(QWidget *parent = 0);
    ~Screensaver();

private:
    void initUI();
    void setDatelayout();
    void setSleeptime();
    void setCenterWidget();
    void updateDate();

    QLabel *dateOfWeek;
    QLabel *dateOfLocaltime;
    QLabel *dateOfDay;
    QLabel *dateOfLunar;
    QWidget *centerWidget;
    QLabel *ubuntuKylinlogo;
    QWidget *timeLayout;
    SleepTime *sleepTime;
    QTimer *timer;
    ChineseDate *date;
    QGSettings *settings;
    QString background;

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    void updateTime();
};

#endif // MAINWINDOW_H
