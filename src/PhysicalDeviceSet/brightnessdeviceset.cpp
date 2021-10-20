#include "brightnessdeviceset.h"

int BrightnessDeviceSet::m_value = -1;
int BrightnessDeviceSet::m_maxBrightnessValue = -1;
bool BrightnessDeviceSet::m_isPowerSupply = false;
int BrightnessDeviceSet::m_curBrightnessValue = -1;

BrightnessDeviceSet* BrightnessDeviceSet::instance(QObject *parent)
{
    static BrightnessDeviceSet* _instance = nullptr;
    QMutex mutex;
    mutex.lock();
    if(_instance == nullptr)
        _instance = new BrightnessDeviceSet(parent);
    mutex.unlock();
    return _instance;
}

BrightnessDeviceSet::BrightnessDeviceSet(QObject *parent):
    QObject(parent)
{
    init();
}

void BrightnessDeviceSet::init()
{
    //m_isPowerSupply = true;
    m_isPowerSupply = isPowerSupply();
    if(!m_isPowerSupply){
        qWarning() << "info: [BrightnessDeviceSet][init]: current device not support brightness-change!";
    }
    m_curBrightnessValue = getCurBrightness();
    m_maxBrightnessValue = getMaxBrightness();
    //m_maxBrightnessValue = 19200;
    m_value = m_curBrightnessValue * 1.0 / m_maxBrightnessValue * 100;
}

int BrightnessDeviceSet::getValue()
{
    return m_value;
}

void BrightnessDeviceSet::setValue(int value)
{
    if(!m_isPowerSupply) return;
    if(value == m_value) return;
    if(value > 100) value = 100;
    else if(value < 0) value = 0;
    m_value = value;
    //emit valueChanged(m_value);
    // 调整亮度
    int val = value * 1.0 / 100 * m_maxBrightnessValue;
    setBrightness(val);
}

bool BrightnessDeviceSet::isPowerSupply()
{
    QProcess process;
    QString command = "/usr/lib/ukui-greeter/greeter-backlight-helper --get-support-brightness";

    process.start(command);
    process.waitForFinished(3000);
    QString result = process.readAll();

    return result == "true";
}

int BrightnessDeviceSet::getMaxBrightness()
{
    if(!m_isPowerSupply) return 0;
    QProcess process;
    QString command = "/usr/lib/ukui-greeter/greeter-backlight-helper --get-max-brightness";

    process.start(command);
    process.waitForFinished(3000);
    QString result = process.readAll();
    return result.toInt();
}

int BrightnessDeviceSet::getCurBrightness()
{
    if(!m_isPowerSupply) return 0;
    QProcess process;
    QString command = "/usr/lib/ukui-greeter/greeter-backlight-helper --get-brightness";

    process.start(command);
    process.waitForFinished(3000);
    QString result = process.readAll();
    return result.toInt();
}

void BrightnessDeviceSet::setBrightness(int val)
{
    if(!m_isPowerSupply) return;
    if(val > m_maxBrightnessValue) val = m_maxBrightnessValue;
    else if(val < 100) val = 100;

    QString command = "/usr/lib/ukui-greeter/greeter-backlight-helper --set-brightness " + QString::number(val);

    QProcess process;
    process.start(command);
    process.waitForFinished(3000);
    //QString result = process.readAll();
}
