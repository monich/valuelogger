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

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>

#include <qmath.h>

namespace {
    // UTC times are signiticantly faster to compare.
    // Local times are getting converted to UTC every time
    // QDateTimePrivate::toMSecsSinceEpoch() is called.
    const QByteArray TIMESTAMP_ROLE("timestampUTC");
    const QByteArray VALUE_ROLE("value");
}

Graph::Graph(QQuickItem* parent) :
    QQuickItem(parent),
    m_color(Qt::white),
    m_lineWidth(2),
    m_minValue(0),
    m_maxValue(0),
    m_timestampRole(-1),
    m_valueRole(-1),
    m_model(Q_NULLPTR),
    m_paintedCount(0)
{
    setFlag(ItemHasContents);
    setClip(true);
}

Graph::~Graph()
{
    // It may be too early to delete nodes at this point
    const int n = m_nodes.count();
    QSGNode* const* nodes = m_nodes.constData();
    for (int i = 0; i < n; i++) {
        QSGNode* node = nodes[i];
        if (node->parent()) {
            // Let SG do it
            node->setFlag(QSGNode::OwnedByParent);
        } else {
            delete node;
        }
    }
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
        const int n = m_nodes.count();
        QSGNode* const* nodes = m_nodes.constData();
        for (int i = 0; i < n; i++) {
            QSGGeometryNode* node = (QSGGeometryNode*)nodes[i];
            QSGFlatColorMaterial* m = (QSGFlatColorMaterial*)node->material();
            m->setColor(color);
            node->markDirty(QSGNode::DirtyMaterial);
        }
        emit colorChanged();
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

void Graph::updateSquareGeometry(QSGGeometry::Point2D* v, float x, float y, float size)
{
    const float d = size/2.f;
    const float xmin = x - d;
    const float xmax = xmin + size;
    const float ymin = y - d;
    const float ymax = ymin + size;
    v[0].x = xmin; v[0].y = ymin;
    v[1].x = xmin; v[1].y = ymax;
    v[2].x = xmax; v[2].y = ymax;
    v[3].x = xmax; v[3].y = ymin;
}

void Graph::updateLineGeometry(QSGGeometry::Point2D* v, float x1, float y1, float x2, float y2, float thick)
{
    if (x1 == x2 || y1 == y2) {
        // Vertical or horizontal line
        const bool vertical = (x1 == x2);
        const float d = thick/2;
        const float xmin = vertical ? (x1 - d) : qMin(x1, x2);
        const float xmax = vertical ? (x1 + d) : qMax(x1, x2);
        const float ymin = vertical ? qMax(y1, y2) : (y1 - d);
        const float ymax = vertical ? qMax(y1, y2) : (y1 + d);
        v[0].x = xmin; v[0].y = ymin;
        v[1].x = xmin; v[1].y = ymax;
        v[2].x = xmax; v[2].y = ymax;
        v[3].x = xmax; v[3].y = ymin;
    } else {
        // Rotated rectangle
        const float a = atanf((y2 - y1)/(x2 - x1));
        const float dx = sinf(a) * thick / 2;
        const float dy = cosf(a) * thick / 2;
        v[0].x = x1 + dx; v[0].y = y1 - dy;
        v[1].x = x1 - dx; v[1].y = y1 + dy;
        v[2].x = x2 - dx; v[2].y = y2 + dy;
        v[3].x = x2 + dx; v[3].y = y2 - dy;
    }
}

QSGGeometry* Graph::newSquareGeometry(float x, float y, float size)
{
    QSGGeometry* g = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
    updateSquareGeometry(g->vertexDataAsPoint2D(), x, y, size);
    g->setDrawingMode(GL_TRIANGLE_FAN);
    return g;
}

QSGGeometry* Graph::newLineGeometry(float x1, float y1, float x2, float y2, float thick)
{
    QSGGeometry* g = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
    updateLineGeometry(g->vertexDataAsPoint2D(), x1, y1, x2, y2, thick);
    g->setDrawingMode(GL_TRIANGLE_FAN);
    return g;
}

void Graph::updateSquareNode(QSGGeometryNode* node, float x, float y, float size)
{
    updateSquareGeometry(node->geometry()->vertexDataAsPoint2D(), x, y, size);
    node->markDirty(QSGNode::DirtyGeometry);
}

void Graph::updateLineNode(QSGGeometryNode* node, float x1, float y1, float x2, float y2, float thick)
{
    updateLineGeometry(node->geometry()->vertexDataAsPoint2D(), x1, y1, x2, y2, thick);
    node->markDirty(QSGNode::DirtyGeometry);
}

QSGGeometryNode* Graph::newNode(QSGGeometry* g)
{
    QSGFlatColorMaterial* m = new QSGFlatColorMaterial;
    m->setColor(m_color);
    QSGGeometryNode* node = new QSGGeometryNode();
    node->setGeometry(g);
    node->setMaterial(m);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setFlag(QSGNode::OwnsMaterial);
    node->setFlag(QSGNode::OwnedByParent, false); // We manage those
    return node;
}

bool Graph::lineVisible(float x1, float y1, float x2, float y2, float w, float h)
{
    if ((x1 < 0 && x2 < 0) || (x1 >= w && x2 >= w) ||
        (y1 < 0 && y2 < 0) || (y1 >= h && y2 >= h)) {
        return false;
    }
    // Yeah, some cases will slip through but getting it 100% right
    // won't save us much, so let's keep it simple.
    return true;
}

QSGNode* Graph::updatePaintNode(QSGNode* paintNode, UpdatePaintNodeData*)
{
    int paintedCount = 0;
    if (!m_model || !m_model->rowCount()) {
        qDeleteAll(m_nodes);
        m_nodes.resize(0);
        delete paintNode;
        paintNode = Q_NULLPTR;
    } else {
        if (!paintNode) {
            paintNode = new QSGNode;
        }

        const float w = width();
        const float h = height();
        if (m_model && m_timestampRole >= 0 && m_valueRole >= 0 &&
            w > 0 && h > 0 && m_minValue < m_maxValue &&
            m_minTime.isValid() && m_maxTime.isValid() &&
            m_minTime < m_maxTime) {
            const float timeSpan = m_minTime.msecsTo(m_maxTime);
            const float valueSpan = m_maxValue - m_minValue;

            // Calculate points and figure out if we need squares
            int i;
            bool drawSquares = true;
            const int n = m_model->rowCount();
            const float minDistSquared = 9 * m_lineWidth * m_lineWidth;
            QVector<QPointF> points;
            points.reserve(n);
            for (i = 0; i < n; i++) {
                bool ok;
                const QModelIndex index(m_model->index(i, 0));
                const QDateTime time(m_model->data(index, m_timestampRole).toDateTime());
                const qreal value = m_model->data(index, m_valueRole).toReal(&ok);
                if (ok) {
                    const float x = w * m_minTime.msecsTo(time) / timeSpan;
                    const float y = h * (m_maxValue - value) / valueSpan;
                    if (drawSquares && !points.isEmpty()) {
                        const QPointF& last(points.last());
                        const float lastX = last.x();
                        const float lastY = last.y();
                        if (lineVisible(lastX, lastY, x, y, w, h)) {
                            const float dx = x - lastX;
                            const float dy = y - lastY;
                            const float distSquared = dx * dx + dy * dy;
                            if (distSquared < minDistSquared) {
                                drawSquares = false;
                                DBG("Not drawing squares");
                            }
                        }
                    }
                    points.append(QPointF(x, y));
                }
            }

            // Reuse the existing nodes
            QSGNode* node = paintNode->firstChild();
            const float xmin = -m_lineWidth;
            const float ymin = -m_lineWidth;
            const float xmax = w + m_lineWidth;
            const float ymax = h + m_lineWidth;
            const int np = points.count();
            const QPointF* pa = points.constData();
            float lastX = 0, lastY = 0;
            for (i = 0; i < np; i++) {
                const float x = pa[i].x();
                const float y = pa[i].y();

                if (x >= xmin && x < xmax && y >= ymin && y < ymax) {
                    paintedCount++;
                    if (drawSquares) {
                        const float d = 2 * m_lineWidth;
                        if (node) {
                            updateSquareNode((QSGGeometryNode*)node, x, y, d);
                            node = node->nextSibling();
                        } else {
                            QSGNode* child = newNode(newSquareGeometry(x, y, d));
                            paintNode->appendChildNode(child);
                            m_nodes.append(child);
                        }
                    }
                }

                if (i > 0 && lineVisible(lastX, lastY, x, y, w, h)) {
                    if (node) {
                        updateLineNode((QSGGeometryNode*)node, lastX, lastY, x, y, m_lineWidth);
                        node = node->nextSibling();
                    } else {
                        QSGNode* child = newNode(newLineGeometry(lastX, lastY, x, y, m_lineWidth));
                        paintNode->appendChildNode(child);
                        m_nodes.append(child);
                    }
                }

                lastX = x;
                lastY = y;
            }

            // Drop nodes that we no longer need
            if (node) {
                QSGNode* last;
                // Remove nodes that we no longer need. This may look dangerous
                // but the last unused node must be in m_nodes list.
                do {
                    last = m_nodes.takeLast();
                    paintNode->removeChildNode(last);
                    delete last;
                } while (last != node);
            }
            m_nodes.squeeze();
        } else {
            paintNode->removeAllChildNodes();   // This doesn't delete nodes
            qDeleteAll(m_nodes);                // But this does
            m_nodes.resize(0);
        }

        DBG(m_nodes.count());
    }

    if (m_paintedCount != paintedCount) {
        m_paintedCount = paintedCount;
        Q_EMIT paintedCountChanged();
    }
    return paintNode;
}
