/*
Copyright (c) 2021-2025 Slava Monich <slava@monich.com>

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "graph.h"
#include "debuglog.h"

#include <QPainter>
#include <QPainterPath>
#include <QBrush>
#include <QPen>

#include <qmath.h>

#define PRINTABLE_TIME(d) qPrintable((d).toString(QStringLiteral("dd.MM.yyyyThh:mm")))

namespace {
    // UTC times are signiticantly faster to compare.
    // Local times are getting converted to UTC every time
    // QDateTimePrivate::toMSecsSinceEpoch() is called.
    const QByteArray TIMESTAMP_ROLE("timestampUTC");
    const QByteArray VALUE_ROLE("value");
}

Graph::Graph(QQuickItem* parent) :
    QQuickPaintedItem(parent),
    m_color(Qt::white),
    m_lineWidth(2),
    m_minValue(0),
    m_maxValue(0),
    m_timestampRole(-1),
    m_valueRole(-1),
    m_model(Q_NULLPTR),
    m_nodeMarks(true),
    m_smooth(true),
    m_paintedCount(0)
{
    setClip(true);
    setAntialiasing(true);
}

void Graph::setModel(QObject* model)
{
    QAbstractItemModel* itemModel = qobject_cast<QAbstractItemModel*>(model);
    if (m_model != itemModel) {
        if (m_model) {
            m_model->disconnect(this);
        }
        m_model = itemModel;
        if (m_model) {
            connect(m_model, SIGNAL(destroyed(QObject*)), SLOT(onModelDestroyed()));
            const QHash<int,QByteArray> roleNames(m_model->roleNames());
            const QList<int> roleIds(roleNames.keys());
            m_timestampRole = -1;
            m_valueRole = -1;
            for (int i = roleIds.count() - 1;
                 i >= 0 && (m_timestampRole < 0 || m_valueRole < 0);
                 i--) {
                const int roleId = roleIds.at(i);
                const QByteArray roleName(roleNames.value(roleId));
                if (m_timestampRole < 0 && roleName == TIMESTAMP_ROLE) {
                    m_timestampRole = roleId;
                    DBG(roleName << "=>" << m_timestampRole);
                } else if (m_valueRole < 0 && roleName == VALUE_ROLE) {
                    m_valueRole = roleId;
                    DBG(roleName << "=>" << m_valueRole);
                }
            }
            emit modelChanged();
        }
        update();
    }
}

void Graph::onModelDestroyed()
{
    DBG("model destroyed");
    m_model = Q_NULLPTR;
    emit modelChanged();
}

void Graph::setColor(const QColor& color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
        update();
    }
}

void Graph::setLineWidth(qreal lineWidth)
{
    if (m_lineWidth != lineWidth) {
        m_lineWidth = lineWidth;
        DBG(lineWidth);
        emit lineWidthChanged();
        update();
    }
}

void Graph::setMinValue(qreal value)
{
    if (m_minValue != value) {
        m_minValue = value;
        DBG(value);
        emit minValueChanged();
        update();
    }
}

void Graph::setMaxValue(qreal value)
{
    if (m_maxValue != value) {
        m_maxValue = value;
        DBG(value);
        emit maxValueChanged();
        update();
    }
}

void Graph::setMinTime(const QDateTime& t)
{
    if (m_minTime != t) {
        m_minTime = t;
        DBG(PRINTABLE_TIME(t));
        emit minTimeChanged();
        update();
    }
}

void Graph::setMaxTime(const QDateTime& t)
{
    if (m_maxTime != t) {
        m_maxTime = t;
        DBG(PRINTABLE_TIME(t));
        emit maxTimeChanged();
        update();
    }
}

void Graph::setNodeMarks(bool value)
{
    if (m_nodeMarks != value) {
        m_nodeMarks = value;
        DBG(value);
        emit nodeMarksChanged();
        update();
    }
}

void Graph::setSmooth(bool value)
{
    if (m_smooth != value) {
        m_smooth = value;
        DBG(value);
        emit smoothChanged();
        update();
    }
}

bool Graph::lineVisible(const QPointF& p1, const QPointF& p2, const QRectF& graphRect)
{
    const qreal x1 = p1.x();
    const qreal y1 = p1.y();
    const qreal x2 = p2.x();
    const qreal y2 = p2.y();
    const QPointF topleft(qMin(x1, x2), qMin(y1, y2));

    if (graphRect.contains(topleft)) {
        return true;
    } else {
        const QPointF bottomRight(qMax(x1, x2), qMax(y1, y2));

        return graphRect.contains(bottomRight) ||
            QRectF(topleft, bottomRight).intersects(graphRect);
    }
}

void Graph::paint(QPainter* aPainter)
{
    int paintedCount = 0;
    const QRectF rect(boundingRect());
    const qreal w = rect.width();
    const qreal h = rect.height();

    if (m_model && m_model->rowCount() &&
        m_timestampRole >= 0 && m_valueRole >= 0 &&
        w > 0 && h > 0 && m_minValue < m_maxValue &&
        m_minTime.isValid() && m_maxTime.isValid() &&
        m_minTime < m_maxTime) {
        const qreal timeSpan = m_minTime.msecsTo(m_maxTime);
        const qreal valueSpan = m_maxValue - m_minValue;

        // Calculate points and figure out if we need squares
        int i;
        bool drawSquares = m_nodeMarks;
        const int n = m_model->rowCount();
        const qreal minDistSquared = 9 * m_lineWidth * m_lineWidth;
        QVector<QPointF> points;
        points.reserve(n);
        for (i = 0; i < n; i++) {
            bool ok;
            const QModelIndex index(m_model->index(i, 0));
            const QDateTime time(m_model->data(index, m_timestampRole).toDateTime());
            const qreal value = m_model->data(index, m_valueRole).toReal(&ok);
            if (ok) {
                const qreal x = w * m_minTime.msecsTo(time) / timeSpan;
                const qreal y = h * (m_maxValue - value) / valueSpan;
                const QPointF p(x, y);
                if (drawSquares && !points.isEmpty()) {
                    const QPointF& last(points.last());
                    if (lineVisible(last, p, rect)) {
                        const qreal dx = x - last.x();
                        const qreal dy = y - last.y();
                        const qreal distSquared = dx * dx + dy * dy;
                        if (distSquared < minDistSquared) {
                            drawSquares = false;
                            DBG("Not drawing squares");
                        }
                    }
                }
                points.append(p);
            }
        }

        const qreal xmin = -m_lineWidth;
        const qreal ymin = -m_lineWidth;
        const qreal xmax = w + m_lineWidth;
        const qreal ymax = h + m_lineWidth;
        const int np = points.count();
        const QPointF* pa = points.constData();

        const QBrush brush(m_color);
        QPen pen(m_color);
        pen.setWidth(m_lineWidth);

        const QPointF* prev2 = Q_NULLPTR;

        for (i = 0; i < np; i++) {
            const QPointF* prev = i > 0 ? (pa + (i - 1)) : Q_NULLPTR;
            const QPointF* next = (i + 1) < np ? (pa + (i + 1)) : Q_NULLPTR;
            const QPointF& p = pa[i];
            const qreal x = p.x();
            const qreal y = p.y();

            if (x >= xmin && x < xmax && y >= ymin && y < ymax) {
                paintedCount++;
                if (drawSquares) {
                    const qreal d = 2 * m_lineWidth;
                    const QRectF rect(x - m_lineWidth, y - m_lineWidth, d, d);
                    aPainter->setPen(Qt::NoPen);
                    aPainter->fillRect(rect, brush);
                }
            }

            if (prev && lineVisible(*prev, p, rect)) {
                aPainter->setPen(pen);
                if (m_smooth && prev->x() != x && prev->y() != y) {
                    const qreal L = 0.4;
                    // Initialize control points with the same y as the end points.
                    // For edges and spikes it will remain that way.
                    const qreal dx = x - prev->x();
                    const qreal dy = y - prev->y();
                    const qreal l = qSqrt(dx * dx + dy * dy) * L;
                    QPointF ctlPt1(prev->x() + dx * L, prev->y());
                    QPointF ctlPt2(x - dx * L, y);
                    QPainterPath path;

                    if (prev2 && x != prev2->x() &&
                        (prev->y() - prev2->y()) * (prev->y() - y) < 0) {
                        // prev->y() somewhere is between prev2->y() and y
                        const qreal a = qAtan((y - prev2->y()) / (x - prev2->x()));

                        ctlPt1.setX(prev->x() + l * qCos(a));
                        ctlPt1.setY(prev->y() + l * qSin(a));
                    }

                    if (next && next->x() != x && next->x() != prev->x() &&
                        (y - prev->y()) * (y - next->y()) < 0) {
                        // y is somewhere between prev->y() and next->y()
                        const qreal a = qAtan((next->y() - prev->y()) / (next->x() - prev->x()));

                        ctlPt2.setX(x - l * qCos(a));
                        ctlPt2.setY(y - l * qSin(a));
                    }

#if 0 // Visualization of control points
                    QPen pen1(Qt::white);
                    pen1.setWidth(0);
                    aPainter->setPen(pen1);
                    aPainter->drawLine(*prev, ctlPt1);
                    aPainter->drawLine(ctlPt2, p);
                    aPainter->setPen(pen);
#endif

                    path.moveTo(*prev);
                    path.cubicTo(ctlPt1, ctlPt2, p);
                    aPainter->drawPath(path);

                } else {
                    aPainter->drawLine(*prev, p);
                }
            }

            prev2 = prev;
        }
    }

    if (m_paintedCount != paintedCount) {
        m_paintedCount = paintedCount;
        DBG(paintedCount);
        Q_EMIT paintedCountChanged();
    }
}
