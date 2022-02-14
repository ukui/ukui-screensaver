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

#include <QMouseEvent>
#include <QDebug>

#include <math.h>
#include "swipegesturerecognizer.h"

SwipeGestureRecognizer::SwipeGestureRecognizer(QObject *parent) : QObject(parent)
{
}

bool
SwipeGestureRecognizer::IsValidMove(int dx, int dy)
{
    // The moved distance is to small to count as not just a glitch.
    if ((qAbs(dx) < MINIMUM_DISTANCE) && (qAbs(dy) < MINIMUM_DISTANCE)) {
        return false;
    }

    return true;
}


// virtual
QGesture*
SwipeGestureRecognizer::create(QObject* pTarget)
{
    QGesture *pGesture = new QSwipeGesture(pTarget);
    return pGesture;
}


// virtual
QGestureRecognizer::Result
SwipeGestureRecognizer::recognize(QGesture* pGesture, QObject *pWatched, QEvent *pEvent)
{
    QGestureRecognizer::Result result = QGestureRecognizer::Ignore;
    QSwipeGesture *pSwipe = static_cast<QSwipeGesture*>(pGesture);

    switch(pEvent->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        pSwipe->setProperty("startPoint", pMouseEvent->posF());
#else
        pSwipe->setProperty("startPoint", pMouseEvent->localPos());
#endif
        result = QGestureRecognizer::MayBeGesture;
        //qDebug() << "Swipe gesture started (start point=" <<  pSwipe->property("startPoint").toPointF() <<  ")";
    }
        break;
    case QEvent::MouseButtonRelease: {
        QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
        const QVariant& propValue = pSwipe->property("startPoint");
        QPointF startPoint = propValue.toPointF();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        QPointF endPoint = pMouseEvent->posF();
#else
        QPointF endPoint = pMouseEvent->localPos();
#endif

        // process distance and direction
        int dx = endPoint.x() - startPoint.x();
        int dy = endPoint.y() - startPoint.y();

        //bugfix: startPoint.isNull because we sometimes get false events with startpoint 0 -> wrong swipe detected!
        if ( (!IsValidMove(dx, dy)) || ( startPoint.isNull() ) ) {
            // Just a click, so no gesture.
            result = QGestureRecognizer::CancelGesture;
            //qDebug("Swipe gesture canceled");
        } else {
            // Compute the angle.
            // qDebug() << " startPoint= " << startPoint << " endPoint=" << endPoint << " dx=" << dx << " dy=" << dy;
            Q_EMIT onSwipeGesture(dx, dy);
            qreal angle = ComputeAngle(dx, dy);
            pSwipe->setSwipeAngle(angle);
            result = QGestureRecognizer::FinishGesture;
            //qDebug("Swipe gesture finished");
        }
    }
        break;
    default:
        break;
    }

    return result;
}

void
SwipeGestureRecognizer::reset(QGesture *pGesture)
{
    pGesture->setProperty("startPoint", QVariant(QVariant::Invalid));
    parent::reset(pGesture);
}

qreal
SwipeGestureRecognizer::ComputeAngle(int dx, int dy)
{
    double PI = 3.14159265;

    // Need to convert from screen coordinates direction
    // into classical coordinates direction.
    dy = -dy;

    double result = atan2((double)dy, (double)dx) ;
    result = (result * 180) / PI;

    // Always return positive angle.
    if (result < 0) {
        result += 360;
    }

    return result;
}



/*
==========================================================================
*/

QSwipeGesture::SwipeDirection
SwipeGestureUtil::GetHorizontalDirection(QSwipeGesture *pSwipeGesture)
{
    qreal angle = pSwipeGesture->swipeAngle();
    if (0 <= angle && angle <= 45) {
        return QSwipeGesture::Right;
    }

    if (135 <= angle && angle <= 225) {
        return QSwipeGesture::Left;
    }

    if (315 <= angle && angle <= 360) {
        return QSwipeGesture::Right;
    }

    return QSwipeGesture::NoDirection;
}

QSwipeGesture::SwipeDirection
SwipeGestureUtil::GetVerticalDirection(QSwipeGesture *pSwipeGesture)
{
    qreal angle = pSwipeGesture->swipeAngle();

    if (45 < angle && angle < 135) {
        return QSwipeGesture::Up;
    }

    if (225 < angle && angle < 315) {
        return QSwipeGesture::Down;
    }

    return QSwipeGesture::NoDirection;
}
