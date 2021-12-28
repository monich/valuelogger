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

#include "dashline.h"
#include "debuglog.h"

#include <QSGNode>
#include <QSGFlatColorMaterial>

DashLine::DashLine(QQuickItem* parent) :
    QQuickItem(parent),
    m_color(Qt::white),
    m_dashSize(DashSizeAuto)
{
    setFlag(ItemHasContents);
}

DashLine::~DashLine()
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

void DashLine::setColor(const QColor& color)
{
    if (m_color != color) {
        m_color = color;
        const int n = m_nodes.count();
        QSGNode* const* nodes = m_nodes.constData();
        for (int i = 0; i < n; i++) {
            QSGGeometryNode* node = (QSGGeometryNode*)nodes[i];
            QSGFlatColorMaterial* m = (QSGFlatColorMaterial*)node->material();
            m->setColor(color);
        }
        update();
        emit colorChanged();
    }
}

void DashLine::setDashSize(qreal size)
{
    if (m_dashSize != size) {
        m_dashSize = size;
        DBG(size);
        emit dashSizeChanged();
        update();
    }
}

void DashLine::updateRectGeometry(QSGGeometry::Point2D* v, float x, float y, float w, float h)
{
    const float xmin = x;
    const float xmax = x + w;
    const float ymin = y;
    const float ymax = y + h;
    v[0].x = xmin; v[0].y = ymin;
    v[1].x = xmin; v[1].y = ymax;
    v[2].x = xmax; v[2].y = ymax;
    v[3].x = xmax; v[3].y = ymin;
}

QSGGeometry* DashLine::newRectGeometry(float x, float y, float w, float h)
{
    QSGGeometry* g = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
    updateRectGeometry(g->vertexDataAsPoint2D(), x, y, w, h);
    g->setDrawingMode(GL_TRIANGLE_FAN);
    return g;
}

void DashLine::updateRectNode(QSGGeometryNode* node, float x, float y, float w, float h)
{
    updateRectGeometry(node->geometry()->vertexDataAsPoint2D(), x, y, w, h);
    node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
}

QSGGeometryNode* DashLine::newNode(QSGGeometry* g)
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

QSGNode* DashLine::updatePaintNode(QSGNode* paintNode, UpdatePaintNodeData*)
{
    if (m_dashSize != 0 && width() > 0 && height() > 0) {
        if (!paintNode) paintNode = new QSGNode;
        updateNodes(paintNode, &m_nodes);
        m_nodes.squeeze();
        DBG(m_nodes.count());
        return paintNode;
    } else {
        qDeleteAll(m_nodes);
        m_nodes.resize(0);
        delete paintNode;
        return Q_NULLPTR;
    }
}
