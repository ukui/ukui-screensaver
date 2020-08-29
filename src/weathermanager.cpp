#include "weathermanager.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QTimer>
#include <QString>
#include <QEventLoop>
#include <QDebug>
#include <QtNetwork>
#include <QPixmap>

const QString weatherServerAddr = "http://service.ubuntukylin.com:8001/weather/api/3.0/heweather_data_s6/";
const QString defaultCityId = "101030100"; //"101030100" 默认天津
const int weatherReqInterval = 1000 * 60 * 20; //定时更新天气,和麒麟天气保持一致

const QByteArray schemaWeather = "org.china-weather-data.settings";

static const QMap<QString, QString> weatherMap {
    {"NA", "55"},
    {"晴", "0"},
    {"多云", "1"},
    {"阴", "2"},
    {"阵雨", "3"},
    {"雷阵雨", "4"},
    {"小雨", "7"},
    {"中雨", "8"},
    {"大雨", "9"},
    {"中到大雨", "9"},
    {"雪", "13"},
};

WeatherManager::WeatherManager(QObject *parent) : QObject(parent)
{
    //初始化m_net_manager请求天气
    m_net_manager = new QNetworkAccessManager(this);
    QObject::connect(m_net_manager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(replyFinished(QNetworkReply*)));

    m_timer = new QTimer(this);
    m_timer->setInterval(weatherReqInterval);

    m_local_weather_info = new LocalWeatherInfo(this);

    connect(m_timer, &QTimer::timeout, this, &WeatherManager::weatherRequest);
}

void WeatherManager::getWeather()
{
    weatherRequest();
    if (m_timer != NULL)
    {
        m_timer->start();
    }
    return;
}

void WeatherManager::weatherRequest()
{
    if (updateLocation())
        return;
    QNetworkRequest request(QUrl(weatherServerAddr + m_city_id + "/"));
    m_net_manager->get(request);
}

bool WeatherManager::updateLocation()
{
    //更新城市信息 未实现  麒麟天气提供gsettings，完成后对接
    if (QGSettings::isSchemaInstalled(schemaWeather)) {
        m_settings = new QGSettings(schemaWeather,"",this); //org/ukui/indicator-china-weather path
        if(getLogcalWeather())
        {
            emit onWeatherUpdate(m_local_weather_info->getCityName(),
                                 m_local_weather_info->getCondText(),
                                 m_local_weather_info->getTemperature());
            return true;
        }
        m_city_id = getLogcalCityId();
    }

    if (m_city_id.isEmpty())
        m_city_id = defaultCityId;

    return false;
}

bool WeatherManager::getLogcalWeather()
{
    if (m_settings != nullptr)
    {
        //"1920-08-27 10:17:42,101310204,澄迈,小雨,95%，25℃,北风,1级,"  时间，城市编码，城市名称，天气，湿度，温度，风向，风力
        QString weather = m_settings->get("weather").toString();
        QStringList weatherInfoList = weather.split(",");
        if (weatherInfoList.size() < 9)
            return false;
        m_local_weather_info->setTime(weatherInfoList.at(0));
        if(!m_local_weather_info->isTimeValid())
            return false;

        m_local_weather_info->setCityId(weatherInfoList.at(1));
        m_local_weather_info->setCityName(weatherInfoList.at(2));
        m_local_weather_info->setCondText(weatherInfoList.at(3));
        m_local_weather_info->setAirHumidity(weatherInfoList.at(4));
        m_local_weather_info->setTemperature(weatherInfoList.at(5));
        m_local_weather_info->setWindDirection(weatherInfoList.at(6));
        m_local_weather_info->setWindForce(weatherInfoList.at(7));
        return true;
    }

    return false;
}

QString WeatherManager::getLogcalCityId()
{
    if (m_settings != nullptr) {
        QString citys = m_settings->get("citylist").toString();//"101010100,"
        if (citys.isEmpty())
            return "";

        QStringList cityList = citys.split(",");
        if (cityList.size() >= 1)
        {
            QString s = cityList.at(0);
            qDebug() << "local city id = " << s;
            return s;
        }
    }
    return "";
}

void WeatherManager::replyFinished(QNetworkReply *reply)
{
    //注：天气信息只解析了锁屏需要展示的部分
    QByteArray BA;
    QJsonDocument JD;
    QJsonParseError JPE;

    BA = reply->readAll();
    //    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    //    QString all = codec->toUnicode(BA);
    //    qDebug() << "reply is:" << all;

    JD = QJsonDocument::fromJson(BA, &JPE);
    if (JPE.error == QJsonParseError::NoError)
    {
        if (JD.isObject())
        {
            QJsonObject kylinWeatherObj = JD.object().value("KylinWeather").toObject();

            QString nowWeather = kylinWeatherObj.value("weather").toObject().value("now").toString();
            m_city_name = kylinWeatherObj.value("weather").toObject().value("location").toString();
            QStringList nowList = nowWeather.split(",");
            for(QString now : nowList)
            {
                if(now.contains("cond_txt")) {
                    m_cond_txt = now.mid(9);
                }

                if (now.contains("tmp")){
                    m_temperature = now.mid(4);
                }
            }
            emit onWeatherUpdate(m_city_name, m_cond_txt, m_temperature);
        }
    } else {
        qWarning() << "get weather info error : " << JPE.errorString();
    }

    reply->deleteLater();
}

QPixmap WeatherManager::getWeatherIcon()
{
    return getWeatherIcon(m_cond_txt);
}

QPixmap WeatherManager::getWeatherIcon(QString cond)
{
    if (cond.isEmpty())
    {
        qWarning() << "cond info is unknown";
        return QPixmap(":/image/assets/weather/55.png").scaled(32,32);
    }

    //根据m_cond_txt
    QString numStr = weatherMap.value(cond);
    if (!numStr.isEmpty()) {
        qDebug() << "----------------numStr=" + numStr;
        return QPixmap(":/image/assets/weather/" + numStr +".png").scaled(32,32);
    }

    return QPixmap(":/image/assets/weather/55.png").scaled(32,32);
}

LocalWeatherInfo::LocalWeatherInfo(QObject *parent)
{
}

bool LocalWeatherInfo::isTimeValid()
{
    if (m_update_time != nullptr && !m_update_time.isEmpty())
    {
        QString strBuffer;
        QDateTime time = QDateTime::fromString(m_update_time, "yyyy-MM-dd hh:mm:ss");
        QDateTime currentTime = QDateTime::currentDateTime();

        if (!time.isValid())
            return false;

        uint timeInterval = currentTime.toTime_t() - time.toTime_t();

        if((timeInterval <= 21 * 60)
                && (timeInterval > 0))
        {
            //麒麟天气更新时间为20分钟，加入1分钟容错机制
            return true;
        }
    }
    return false;
}

void LocalWeatherInfo::setTime(QString time)
{
    m_update_time = time;
}

QString LocalWeatherInfo::getTime()
{
    return m_update_time;
}

void LocalWeatherInfo::setCityId(QString cityId)
{
    m_city_id = cityId;
}

QString LocalWeatherInfo::getCityId()
{
    return m_city_id;
}

void LocalWeatherInfo::setCityName(QString cityName)
{
    m_city_name = cityName;
}

QString LocalWeatherInfo::getCityName()
{
    return m_city_name;
}

void LocalWeatherInfo::setCondText(QString condText)
{
    m_cond_text = condText;
}

QString LocalWeatherInfo::getCondText()
{
    return m_cond_text;
}

void LocalWeatherInfo::setAirHumidity(QString airHumidity)
{
    m_air_humidity = airHumidity;
}

QString LocalWeatherInfo::getAirHumidity()
{
    return m_air_humidity;
}

void LocalWeatherInfo::setTemperature(QString temperature)
{
    m_temperature = temperature;
}

QString LocalWeatherInfo::getTemperature()
{
    return m_temperature;
}

void LocalWeatherInfo::setWindDirection(QString windDirection)
{
    m_wind_direction = windDirection;
}

QString LocalWeatherInfo::getWindDirection()
{
    return m_wind_direction;
}

void LocalWeatherInfo::setWindForce(QString windForce)
{
    m_wind_force = windForce;
}

QString LocalWeatherInfo::getWindForce()
{
    return m_wind_force;
}
