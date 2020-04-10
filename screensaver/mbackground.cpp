#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QMimeDatabase>
#include "mbackground.h"

MBackground::MBackground():
    currentIndex(0)
{
    getAllPixmap();

}

QString MBackground::getCurrent()
{
    if(list.count()<0)
        return "";

    if(currentIndex>=0 && currentIndex<list.count())
        return list.at(currentIndex);

    return "";
}

QString MBackground::getNext()
{
    if(list.count() == 1)
        return list.at(0);

    if(list.count()<=0 || currentIndex>=list.count() ||currentIndex<0)
        return "";

    if(currentIndex == list.count() - 1)
        currentIndex = 0;
    else
        currentIndex++;

    return list.at(currentIndex);
}

QString MBackground::getPrev()
{
    if(list.count() == 1)
        return list.at(0);

    if(list.count()<=0 || currentIndex>=list.count() || currentIndex<0)
        return "";

    if(currentIndex == 0)
        currentIndex = list.count() - 1;
    else
        currentIndex--;

    return list.at(currentIndex);
}

void MBackground::getAllPixmap()
{
    QDomDocument doc;
    QFile file("/usr/share/ukui-background-properties/focal-ubuntukylin-wallpapers.xml");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<file.fileName()<<" open failed";
        return ;
    }

    if (!doc.setContent(&file))
    {
        file.close();
        return ;
    }
    file.close();

    QDomElement root = doc.documentElement();//读取根节点

    QDomNode node = root.firstChild();//读取第一个子节点   QDomNode 节点
    while (!node.isNull())
    {
        QDomElement node1 = node.firstChildElement("filename");
        QString fileName = node1.text();

        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(fileName);
        if(mime.name().startsWith("image/")){
            list.append(fileName);
        }

        node = node.nextSibling();//读取下一个兄弟节点
    }
}
