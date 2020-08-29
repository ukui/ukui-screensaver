#ifndef WEATHERMANAGER_H
#define WEATHERMANAGER_H

#include <QObject>
#include <functional>
#include <QTimer>
#include <QGSettings>

class QNetworkAccessManager;
class QNetworkReply;
class LocalWeatherInfo;

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
    bool updateLocation();//更新位置，从用户设置获取城市信息，如有多个，只取第一个,未对接
    void weatherRequest();

    bool getLogcalWeather();
    QString getLogcalCityId();

private:
    QString m_city_id; // "101030100" 默认天津
    QString m_city_name;
    QString m_cond_txt; //天气条件 晴、阴等
    QString m_temperature;//温度 10、20等

    QNetworkAccessManager *m_net_manager;
    QTimer *m_timer;
    QGSettings *m_settings;

    LocalWeatherInfo *m_local_weather_info;
};

class LocalWeatherInfo : QObject
{
    //"1920-08-27 10:17:42,101310204,澄迈,小雨,95%，25℃,北风,1级,"  时间，城市编码，城市名称，天气，湿度，温度，风向，风力
    Q_OBJECT
public:
    explicit LocalWeatherInfo(QObject *parent = nullptr);

private:
    QString m_update_time;
    QString m_city_id;
    QString m_city_name;
    QString m_cond_text;
    QString m_air_humidity;
    QString m_temperature;
    QString m_wind_direction;
    QString m_wind_force;
public:
    bool isTimeValid();

    void setTime(QString time);
    QString getTime();

    void setCityId(QString cityId);
    QString getCityId();

    void setCityName(QString cityName);
    QString getCityName();

    void setCondText(QString condText);
    QString getCondText();

    void setAirHumidity(QString airHumidity);
    QString getAirHumidity();

    void setTemperature(QString temperature);
    QString getTemperature();

    void setWindDirection(QString windDirection);
    QString getWindDirection();

    void setWindForce(QString windForce);
    QString getWindForce();
};

#endif // WEATHERMANAGER_H
