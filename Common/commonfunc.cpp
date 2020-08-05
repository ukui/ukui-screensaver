#include <QMimeType>
#include <QSettings>
#include <QMimeDatabase>

#include "commonfunc.h"

bool ispicture(QString filepath)
{
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(filepath);
        return mime.name().startsWith("image/");
}

QString getSystemVersion()
{
    QSettings settings("/etc/lsb-release", QSettings::IniFormat);
    QString release = settings.value("DISTRIB_RELEASE").toString();
    QString description = settings.value("DISTRIB_DESCRIPTION").toString();
    if(description.right(3) == "LTS")
        release = release + " LTS";
    return release;
}

QString getSystemDistrib()
{
    QSettings settings("/etc/lsb-release", QSettings::IniFormat);
    QString distribId = settings.value("DISTRIB_ID").toString();
    return distribId;
}

commonFunc::commonFunc()
{

}
