#include "touchscreenset.h"

#include <QMutex>

TouchScreenSet* TouchScreenSet::instance(QObject *parent)
{
    static TouchScreenSet* _instance = nullptr;
    QMutex mutex;
    mutex.lock();
    if(!_instance)
        _instance = new TouchScreenSet(parent);
    mutex.unlock();
    return _instance;
}

TouchScreenSet::TouchScreenSet(QObject *parent):
    QObject(parent)
{
    init();
}

void TouchScreenSet::init()
{

}

int TouchScreenSet::getTouchScreenID()
{

}
