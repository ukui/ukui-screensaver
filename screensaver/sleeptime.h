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
    int setTime();

private:
    QLabel *restTime;
    QList<QLabel *> list;
    QHBoxLayout *layout;
    int sleepTime;

    void init();
    void setSeconds(int seconds);
    void setMinute(int minutes);
};

#endif // SLEEPTIME_H
