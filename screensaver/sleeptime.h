#ifndef SLEEPTIME_H
#define SLEEPTIME_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QList>
#include <QHBoxLayout>

class SleepTime : public QWidget
{
    Q_OBJECT
public:
    explicit SleepTime(QWidget *parent = nullptr);
    ~SleepTime();
    void setTime();

private:
    QList<QLabel *> list;
    QHBoxLayout *layout;
    int sleepTime;
    int localMin;
    int timecount;

    void init();
    void setSeconds(int seconds);
    void setMinute(int minutes);
};

#endif // SLEEPTIME_H
