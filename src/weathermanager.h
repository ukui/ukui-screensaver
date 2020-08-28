#ifndef WEATHERMANAGER_H
#define WEATHERMANAGER_H

#include <QObject>
#include <functional>
#include <QTimer>

class QNetworkAccessManager;
class QNetworkReply;

class WeatherManager : public QObject
{
    Q_OBJECT
public:
    explicit WeatherManager(QObject *parent = nullptr);

Q_SIGNALS:
    void onWeatherUpdate(QString city, QString cond, QString temperature);

private Q_SLOTS:
    void replyFinished(QNetworkReply *);

public:
    void getWeather();
    QPixmap getWeatherIcon();
    QPixmap getWeatherIcon(QString cond);

private:
    void updateLocation();//更新位置，从用户设置获取城市信息，如有多个，只取第一个,未对接
    void weatherRequest();

private:
    QString m_city_id; // "101030100" 默认天津
    QString m_city_name;
    QString m_cond_txt; //天气条件 晴、阴等
    QString m_temperature;//温度 10、20等

    QNetworkAccessManager *m_net_manager;
    QTimer *m_timer;

};

#endif // WEATHERMANAGER_H
