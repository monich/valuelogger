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
    m_model(Q_NULLPTR)
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

void Graph::updateCircleGeometry(QSGGeometry::Point2D* v, float x0, float y0, float r, int n)
{
    v[0].x = x0;
    v[0].y = y0;
    for (int i = 0; i < n; i++) {
        const float theta = i * 2 * M_PI / n;
        v[i + 1].x = x0 + r * cosf(theta);
        v[i + 1].y = y0 + r * sinf(theta);
    }
    v[n + 1] = v[1];
}

void Graph::updateRectGeometry(QSGGeometry::Point2D* v, float x1, float y1, float x2, float y2, float thick)
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

QSGGeometry* Graph::newCircleGeometry(float x, float y, float radius, int n)
{
    QSGGeometry* g = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), n + 2);
    updateCircleGeometry(g->vertexDataAsPoint2D(), x, y, radius, n);
    g->setDrawingMode(GL_TRIANGLE_FAN);
    return g;
}

QSGGeometry* Graph::newRectGeometry(float x1, float y1, float x2, float y2, float thick)
{
    QSGGeometry* g = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
    updateRectGeometry(g->vertexDataAsPoint2D(), x1, y1, x2, y2, thick);
    g->setDrawingMode(GL_TRIANGLE_FAN);
    return g;
}

#define CIRCLE_NODES(r) roundf(8.f * (r))

QSGGeometry* Graph::newNodeGeometry(float x1, float y1, float x2, float y2, float thick)
{
    if (x1 == x2 && y1 == y2) {
        const float r = thick/2.f;
        return newCircleGeometry(x1, y1, r, CIRCLE_NODES(r));
    } else {
        return newRectGeometry(x1, y1, x2, y2, thick);
    }
}

void Graph::updateNodeGeometry(QSGGeometryNode* node, float x1, float y1, float x2, float y2, float thick)
{
    QSGGeometry* g = node->geometry();
    if (x1 == x2 && y1 == y2) {
        // Dot
        const float r = thick/2.f;
        const int n = CIRCLE_NODES(r);
        if (g->vertexCount() == (n + 2)) {
            updateCircleGeometry(g->vertexDataAsPoint2D(), x1, y1, r, n);
            node->markDirty(QSGNode::DirtyGeometry);
        } else {
            node->setGeometry(newCircleGeometry(x1, y1, r, n));
        }
    } else {
        if (g->vertexCount() == 4) {
            updateRectGeometry(g->vertexDataAsPoint2D(), x1, y1, x2, y2, thick);
            node->markDirty(QSGNode::DirtyGeometry);
        } else {
            node->setGeometry(newNodeGeometry(x1, y1, x2, y2, thick));
        }
    }
}

QSGGeometryNode* Graph::newNode(float x1, float y1, float x2, float y2)
{
    QSGFlatColorMaterial* m = new QSGFlatColorMaterial;
    m->setColor(m_color);
    QSGGeometryNode* node = new QSGGeometryNode;
    node->setGeometry(newNodeGeometry(x1, y1, x2, y2, m_lineWidth));
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
    if (!m_model || !m_model->rowCount()) {
        qDeleteAll(m_nodes);
        m_nodes.resize(0);
        delete paintNode;
        return Q_NULLPTR;
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
            float lastX = 0, lastY = 0;

            // Reuse the existing nodes
            QSGNode* node = paintNode->firstChild();
            const int n = m_model->rowCount();
            bool firstNode = true;
            for (int i = 0; i < n; i++) {
                bool ok;
                const QModelIndex index(m_model->index(i, 0));
                const QDateTime time(m_model->data(index, m_timestampRole).toDateTime());
                const qreal value = m_model->data(index, m_valueRole).toReal(&ok);
                if (ok) {
                    const float x = w * m_minTime.msecsTo(time) / timeSpan;
                    const float y = h * (m_maxValue - value) / valueSpan;
                    if (!firstNode && lineVisible(lastX, lastY, x, y, w, h)) {
                        if (node) {
                            updateNodeGeometry((QSGGeometryNode*)node, lastX, lastY, x, y, m_lineWidth);
                            node = node->nextSibling();
                        } else {
                            node = newNode(lastX, lastY, x, y);
                            paintNode->appendChildNode(node);
                            m_nodes.append(node);
                            node = Q_NULLPTR;
                        }
                    }
                    firstNode = false;
                    lastX = x;
                    lastY = y;
                }
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
        return paintNode;
    }
}
