#ifndef SESSIONWATCHER_H
#define SESSIONWATCHER_H

#include <QObject>

class SessionWatcher : public QObject
{
    Q_OBJECT
public:
    explicit SessionWatcher(QObject *parent = nullptr);

Q_SIGNALS:
    void sessionIdle();

private Q_SLOTS:
    void onStatusChanged(unsigned int status);
};

#endif // SESSIONWATCHER_H
