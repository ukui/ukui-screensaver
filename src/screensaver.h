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
    SAVER_IMAGE,
    SAVER_DEFAULE
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
    explicit ScreenSaver( QObject *parent=nullptr);
    explicit ScreenSaver(const ScreenSaver &screensaver);
    ScreenSaver(ScreenSaver &&screensaver) noexcept;
    bool exists();
    void startSwitchImages();
    void stopSwitchImages();
    bool timerStatus();
    friend QDebug &operator<<(QDebug stream, const ScreenSaver &screensaver);
};

Q_DECLARE_METATYPE(ScreenSaver)

QDebug &operator<<(QDebug stream, const ScreenSaver &screensaver);

#endif // SCREENSAVER_H
