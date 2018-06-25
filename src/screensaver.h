#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QObject>
#include <QTimer>
#include <QDebug>

enum SaverMode
{
    SAVER_BLANK_ONLY = 0,
    SAVER_RANDOM,
    SAVER_SINGLE,
    SAVER_IMAGE
};

enum TransitionEffect
{
    TRANSITION_NONE,
    TRANSITION_FADE_IN_OUT
};

class ScreenSaver : public QObject
{
    Q_OBJECT
public:
    SaverMode           mode;
    //path is a directory or a file path if mode is SAVER_IMAGE
    QString             path;
    QString             lastPath;

    //for images saver
    TransitionEffect    effect;
    int                 interval;

private:
    int                 imageIndex;
    QTimer              *timer;
    QStringList         imagePaths;

Q_SIGNALS:
    void imagePathChanged(const QString &path);

public:
    ScreenSaver(QObject *parent=nullptr);
    ScreenSaver(const ScreenSaver &screensaver);
    ScreenSaver(ScreenSaver &&screensaver) noexcept;
    void startSwitchImages();
    void stopSwitchImages();
    bool timerStatus();
    friend QDebug &operator<<(QDebug stream, const ScreenSaver &screensaver);
};

Q_DECLARE_METATYPE(ScreenSaver)

QDebug &operator<<(QDebug stream, const ScreenSaver &screensaver);

#endif // SCREENSAVER_H
