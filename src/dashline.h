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

#ifndef DASHLINE_H
#define DASHLINE_H

#include <QColor>
#include <QQuickItem>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QVector>

class DashLine : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal dashSize READ getDashSize WRITE setDashSize NOTIFY dashSizeChanged)
    Q_ENUMS(Constants)

public:
    enum Constants {
        DashSizeAuto = -1
    };

    ~DashLine();

    const QColor& getColor() const { return m_color; }
    void setColor(const QColor& color);

    qreal getDashSize() const { return m_dashSize; }
    void setDashSize(qreal size);

protected:
    DashLine(QQuickItem* parent);

    virtual void updateNodes(QSGNode* paintNode, QVector<QSGNode*>* nodes) = 0;
    QSGNode* updatePaintNode(QSGNode* node, UpdatePaintNodeData* data) Q_DECL_OVERRIDE;

    static void updateRectGeometry(QSGGeometry::Point2D* v, float x, float y, float w, float h);
    static void updateRectNode(QSGGeometryNode* node, float x, float y, float w, float h);
    static QSGGeometry* newRectGeometry(float x, float y, float w, float h);
    QSGGeometryNode* newNode(QSGGeometry* g);

signals:
    void colorChanged();
    void dashSizeChanged();

private:
    QColor m_color;
    qreal m_dashSize;
    QVector<QSGNode*> m_nodes;
};

#endif // DASHLINE_H
