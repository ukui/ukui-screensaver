#ifndef MBACKGROUND_H
#define MBACKGROUND_H

#include <QList>
#include <QString>
#include <QGSettings>

class MBackground
{
public:
    MBackground();
    QString getCurrent();
    QString getNext();
    QString getPrev();

private:
    void getAllPixmap();

    QStringList list;
    int currentIndex;

};

#endif // MBACKGROUND_H
