#ifndef BRIGHTNESSDEVICESET_H
#define BRIGHTNESSDEVICESET_H

#include <QObject>
#include <QMutex>
#include <QProcess>
#include <QDebug>

class BrightnessDeviceSet : public QObject
{
    Q_OBJECT
public:
    static BrightnessDeviceSet* instance(QObject *parent = nullptr);

public Q_SLOTS:
    // val from 0 to 100 change
    void setValue(int value);
    int getValue();

private:
    BrightnessDeviceSet(QObject *parent = nullptr);
    void init();
    bool isPowerSupply();
    int getMaxBrightness();
    int getCurBrightness();
    // val from 0 to MaxBrightness change
    void setBrightness(int val);

Q_SIGNALS:
    // val form  to 100 change
    //void valueChanged(int val);

private:
    static int m_value;
    static int m_maxBrightnessValue;
    static int m_curBrightnessValue;
    static bool m_isPowerSupply;
};

#endif // BRIGHTNESSDEVICESET_H
