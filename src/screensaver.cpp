/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
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

bool ScreenSaver::exists()
{
    switch(mode)
    {
    case SAVER_BLANK_ONLY:
        return true;
    case SAVER_RANDOM:
    case SAVER_SINGLE:
        return QFile(path).exists();
    case SAVER_IMAGE:
        return QDir(path).exists();
    case SAVER_DEFAULT:
	return true;
    }
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
    QString modes[] = {"blank-only", "random", "single", "image","default"};
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
    case SAVER_DEFAULT:
	break;
    }

    return debug.maybeSpace();
}

