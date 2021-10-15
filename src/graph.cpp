/*
Copyright (c) 2021 Slava Monich <slava@monich.com>

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
#include <QBrush>
#include <QPen>

#include <qmath.h>

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
        DBG(t);
        emit minTimeChanged();
        update();
    }
}

void Graph::setMaxTime(const QDateTime& t)
{
    if (m_maxTime != t) {
        m_maxTime = t;
        DBG(t);
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

bool Graph::lineVisible(qreal x1, qreal y1, qreal x2, qreal y2, qreal w, qreal h)
{
    if ((x1 < 0 && x2 < 0) || (x1 >= w && x2 >= w) ||
        (y1 < 0 && y2 < 0) || (y1 >= h && y2 >= h)) {
        return false;
    }
    // Yeah, some cases will slip through but getting it 100% right
    // won't save us much, so let's keep it simple.
    return true;
}

void Graph::paint(QPainter* aPainter)
{
    int paintedCount = 0;
    const qreal w = width();
    const qreal h = height();

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
                if (drawSquares && !points.isEmpty()) {
                    const QPointF& last(points.last());
                    const qreal lastX = last.x();
                    const qreal lastY = last.y();
                    if (lineVisible(lastX, lastY, x, y, w, h)) {
                        const qreal dx = x - lastX;
                        const qreal dy = y - lastY;
                        const qreal distSquared = dx * dx + dy * dy;
                        if (distSquared < minDistSquared) {
                            drawSquares = false;
                            DBG("Not drawing squares");
                        }
                    }
                }
                points.append(QPointF(x, y));
            }
        }

        const qreal xmin = -m_lineWidth;
        const qreal ymin = -m_lineWidth;
        const qreal xmax = w + m_lineWidth;
        const qreal ymax = h + m_lineWidth;
        const int np = points.count();
        const QPointF* pa = points.constData();
        qreal lastX = 0, lastY = 0;

        const QBrush brush(m_color);
        QPen pen(m_color);
        pen.setWidth(m_lineWidth);

        for (i = 0; i < np; i++) {
            const qreal x = pa[i].x();
            const qreal y = pa[i].y();

            if (x >= xmin && x < xmax && y >= ymin && y < ymax) {
                paintedCount++;
                if (drawSquares) {
                    const qreal d = 2 * m_lineWidth;
                    const QRectF rect(x - m_lineWidth, y - m_lineWidth, d, d);
                    aPainter->setPen(Qt::NoPen);
                    aPainter->fillRect(rect, brush);
                }
            }

            if (i > 0 && lineVisible(lastX, lastY, x, y, w, h)) {
                aPainter->setPen(pen);
                aPainter->drawLine(lastX, lastY, x, y);
            }

            lastX = x;
            lastY = y;
        }
    }

    if (m_paintedCount != paintedCount) {
        m_paintedCount = paintedCount;
        DBG(paintedCount);
        Q_EMIT paintedCountChanged();
    }
}
