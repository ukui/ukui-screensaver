#ifndef FLIGHTMODESET_H
#define FLIGHTMODESET_H
/*!
 *@brief 设置飞行模式
 * 本想在登录界面加载时 提供实体键 飞行模式的功能
 * 后来发现有效。但是飞行模式具体设置是谁负责管理的 还未知。
 * 最初实现的策略是通过 nmcli networking off 的方式设置飞行模式。
 */


#include <QObject>
#include <QMutex>

class FlightModeSet : public QObject
{
    Q_OBJECT
public:
    static FlightModeSet* instance(QObject *parent = nullptr);

private:
    FlightModeSet(QObject *parent = nullptr);

private:
    static bool m_isFlightModeOpen;

};

#endif // FLIGHTMODESET_H
