#include "flightmodeset.h"

bool FlightModeSet::m_isFlightModeOpen = false;

FlightModeSet* FlightModeSet::instance(QObject *parent)
{
    static FlightModeSet* _instance = nullptr;
    QMutex mutex;
    mutex.lock();
    if(_instance == nullptr)
        _instance = new FlightModeSet(parent);
    mutex.unlock();
    return _instance;
}

FlightModeSet::FlightModeSet(QObject *parent):
    QObject(parent)
{

}
