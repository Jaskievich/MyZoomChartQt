/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chartview.h"
#include <QtGui/QMouseEvent>
#include <QScrollBar>
#include <QMessageBox>
#include <QDateTimeAxis>
#include <QValueAxis>

ChartView::ChartView(QChart *chart, QWidget *parent) :
    QChartView(chart, parent),
    m_isTouching(false),
    old_value_slide(0),
    kx(1.0)
{
 //   setRubberBand(QChartView::RectangleRubberBand);

    setRubberBand(QChartView::HorizontalRubberBand);

    setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    QChartView::horizontalScrollBar()->disconnect();
    connect(QChartView::horizontalScrollBar(), &QScrollBar::valueChanged, this, &ChartView::valueChangedScroll);
    dx_val_begin = 0;

}

void ChartView::valueChangedScroll(int value)
{
    int dx = value - old_value_slide;
    dx_val_begin += dx;
    chart()->scroll(dx, 0);
    old_value_slide = value;
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    if( event->delta() > 0)     chart()->zoomIn(); else chart()->zoomOut();
}


bool ChartView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin) {
        // By default touch events are converted to mouse events. So
        // after this event we will get a mouse event also but we want
        // to handle touch events as gestures only. So we need this safeguard
        // to block mouse events that are actually generated from touch.
        m_isTouching = true;

        // Turn off animations when handling gestures they
        // will only slow us down.
        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (m_isTouching)
        return;
    QChartView::mousePressEvent(event);
    frame_mouse.setLeft(event->x());
    frame_mouse.setTop(event->y());
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isTouching)
        return;
    QChartView::mouseMoveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isTouching)
        m_isTouching = false;
    // Because we disabled animations when touch event was detected
    // we must put them back on.
    //    chart()->setAnimationOptions(QChart::SeriesAnimations);
    frame_mouse.setRight(event->x());
    frame_mouse.setBottom(event->y());
    QChartView::mouseReleaseEvent(event);

    if( frame_mouse.width() == 1 ) return;
    qreal _kx = chart()->plotArea().width()/(frame_mouse.width());
    dx_val_begin = static_cast<int>(dx_val_begin*(_kx)+(frame_mouse.left() - chart()->plotArea().left())*_kx);
    kx *= _kx;
    int width_range = static_cast<int>(chart()->plotArea().width()*(kx - 1));
    int right_rng = width_range - dx_val_begin;
    int left_rng = -dx_val_begin;
    QChartView::horizontalScrollBar()->setRange(left_rng,  right_rng);
    old_value_slide = 0;
}

//![1]
void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
//![1]
    case Qt::Key_Left:{
         chart()->scroll(-10, 0);
//        int val = QChartView::horizontalScrollBar()->value();
//        if(val > QChartView::horizontalScrollBar()->minimum() ){
//            QChartView::horizontalScrollBar()->setValue(QChartView::horizontalScrollBar()->value() - 10);
//        }
        break;
    }
    case Qt::Key_Right:{
        chart()->scroll(10, 0);
//        int val = QChartView::horizontalScrollBar()->value();
//        if( val < QChartView::horizontalScrollBar()->maximum() ){
//            QChartView::horizontalScrollBar()->setValue(QChartView::horizontalScrollBar()->value() + 10);
//        }
        break;
    }
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    case Qt::Key_Escape:
        chart()->zoomReset();
        kx = 1.0;
      //  chart()->scroll(-dx_val_begin, 0);
        dx_val_begin = 0;
        QChartView::horizontalScrollBar()->setRange(0, 0);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void ChartView::resizeEvent(QResizeEvent *event)
{
    QChartView::resizeEvent(event);

}
