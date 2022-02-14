/**
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/

#ifndef SWIPEGESTURERECOGNIZER_H
#define SWIPEGESTURERECOGNIZER_H

#include <QGestureRecognizer>
#include <QSwipeGesture>
#include <QWidget>
#include "kylinnm.h"

/**
 * @brief The SwipeGestureRecognizer class
 * needed because the defalt implementation of swipe is a little bit odd: it only triggers on 3 finger swipes! (5.3)
 *
 * http://developer.android.com/design/patterns/gestures.html
 * http://qt-project.org/doc/qt-5/gestures-overview.html
 */
class SwipeGestureRecognizer :public QObject, public QGestureRecognizer
{
    Q_OBJECT
public:
    SwipeGestureRecognizer(QObject *parent = nullptr);
private:
    static const int MINIMUM_DISTANCE = 10;

    typedef QGestureRecognizer parent;

    bool IsValidMove(int dx, int dy);

    qreal ComputeAngle(int dx, int dy);

    virtual QGesture* create(QObject* pTarget);

    virtual QGestureRecognizer::Result recognize(QGesture* pGesture, QObject *pWatched, QEvent *pEvent);

    void reset (QGesture *pGesture);
signals:
    void onSwipeGesture(int dx, int dy);
};


class SwipeGestureUtil {
public:
    static QSwipeGesture::SwipeDirection GetHorizontalDirection(QSwipeGesture *pSwipeGesture);
    static QSwipeGesture::SwipeDirection GetVerticalDirection(QSwipeGesture *pSwipeGesture);
};

#endif // SWIPEGESTURERECOGNIZER_H
