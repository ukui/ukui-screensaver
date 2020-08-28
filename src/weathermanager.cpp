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

static const QString weatherServerAddr = "http://service.ubuntukylin.com:8001/weather/api/3.0/heweather_data_s6/";
static const QString defaultCityId = "101030100"; //"101030100" 默认天津
static const int weatherReqInterval = 1000 * 60 * 30; //定时更新天气

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

    connect(m_timer, &QTimer::timeout, this, &WeatherManager::weatherRequest);
}

void WeatherManager::getWeather()
{
    weatherRequest();
    if (m_timer != NULL)
    {
        m_timer->start();
        qDebug() << "-------------------getWeather";
    }
    return;
}

void WeatherManager::weatherRequest()
{
    updateLocation();
    QNetworkRequest request(QUrl(weatherServerAddr + m_city_id + "/"));
    m_net_manager->get(request);
}

void WeatherManager::updateLocation()
{
    //更新城市信息 未实现  麒麟天气提供gsettings，完成后对接

    if (m_city_id.isEmpty())
        m_city_id = defaultCityId;
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
