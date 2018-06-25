#include "screensaver.h"
#include <QFileInfo>
#include <QDir>
#include <QImageReader>

ScreenSaver::ScreenSaver(QObject *parent)
    : QObject(parent),
      imageIndex(0),
      timer(nullptr)
{

}

ScreenSaver::ScreenSaver(const ScreenSaver &screensaver)
    : mode(screensaver.mode),
      path(screensaver.path),
      effect(screensaver.effect),
      interval(screensaver.interval),
      imageIndex(screensaver.imageIndex),
      timer(screensaver.timer),
      imagePaths(screensaver.imagePaths)
{
}


ScreenSaver::ScreenSaver(ScreenSaver &&screensaver) noexcept
    : mode(screensaver.mode),
      path(screensaver.path),
      effect(screensaver.effect),
      interval(screensaver.interval),
      imageIndex(screensaver.imageIndex),
      timer(screensaver.timer),
      imagePaths(screensaver.imagePaths)
{
}

void ScreenSaver::startSwitchImages()
{
    qDebug() << "ScreenSaver::startSwitchImages";
    if(mode != SAVER_IMAGE)
        return;
    QFileInfo fileInfo(path);
    if(fileInfo.isFile())
        return;
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    if(fileInfo.isDir()) {
        QDir dir(path);
        QStringList files = dir.entryList(QDir::Files | QDir::Readable);
        for(QString file : files) {
            fileInfo.setFile(file);
            QString suffix = fileInfo.suffix();
            if(formats.contains(suffix.toUtf8()))
                imagePaths.push_back(path + "/" + file);
        }
        if(!imagePaths.empty()) {
            path = imagePaths[0];
            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, [&]{
                imageIndex = (imageIndex + 1) % imagePaths.size();
                path = imagePaths[imageIndex];
                lastPath = imagePaths[imageIndex - 1 < 0 ? imagePaths.size() - 1 : imageIndex - 1];
                Q_EMIT imagePathChanged(path);
            });
            timer->start(interval * 1000);
            Q_EMIT imagePathChanged(path);
        }
    }
}

void ScreenSaver::stopSwitchImages()
{
    if(timer && timer->isActive())
        timer->stop();
}

bool ScreenSaver::timerStatus()
{
    return timer->isActive();
}

QDebug &operator<<(QDebug debug, const ScreenSaver &screensaver)
{
    QString modes[] = {"blank-only", "random", "single", "image"};
    QString effects[] = {"none", "fade-in-out"};
    debug.nospace()<< "screensaver: "<< modes[screensaver.mode];
    switch(screensaver.mode) {
    case SAVER_BLANK_ONLY:
        break;
    case SAVER_RANDOM:
    case SAVER_SINGLE:
        debug.nospace() << screensaver.path;
        break;
    case SAVER_IMAGE:
        debug.nospace() << screensaver.path << effects[screensaver.effect] << screensaver.interval;
    }

    return debug.maybeSpace();
}

