#ifndef TOUCHSCREENSET_H
#define TOUCHSCREENSET_H

#include <QObject>

class TouchScreenSet : public QObject
{
    Q_OBJECT
public:
    TouchScreenSet *instance(QObject* parent = nullptr);
    void init();
    int getTouchScreenID();

private:
    TouchScreenSet(QObject *parent = nullptr);
};

#endif // TOUCHSCREENSET_H
