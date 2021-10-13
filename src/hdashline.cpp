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

#include "hdashline.h"
#include "debuglog.h"

#include <QSGNode>

void HDashLine::updateNodes(QSGNode* paintNode, QVector<QSGNode*>* nodes)
{
    const float w = width();
    const float h = height();

    // Update the existing nodes
    float x;
    QSGNode* node = paintNode->firstChild();
    const float dashSize = (getDashSize() < 0) ? h : getDashSize();
    const float dashSpace = dashSize;
    const float dashStep = dashSize + dashSpace;
    for (x = 0; node && x < w; x += dashStep) {
        updateRectNode((QSGGeometryNode*)node, x, 0, qMin(dashSize, w - x), h);
        node = node->nextSibling();
    }

    // Create new ones
    for (;x < w; x += dashStep) {
        QSGNode* child = newNode(newRectGeometry(x, 0, qMin(dashSize, w - x), h));
        paintNode->appendChildNode(child);
        nodes->append(child);
    }

    if (node) {
        QSGNode* last;
        // Remove nodes that we no longer need. This may look dangerous
        // but the last unused node must be in m_nodes list.
        do {
            last = nodes->takeLast();
            paintNode->removeChildNode(last);
            delete last;
        } while (last != node);
    }
}
