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

#ifndef GRAPH_H
#define GRAPH_H

#include <QColor>
#include <QDateTime>
#include <QQuickPaintedItem>
#include <QAbstractItemModel>

class Graph : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(qreal minValue READ minValue WRITE setMinValue NOTIFY minValueChanged)
    Q_PROPERTY(qreal maxValue READ maxValue WRITE setMaxValue NOTIFY maxValueChanged)
    Q_PROPERTY(QDateTime minTime READ minTime WRITE setMinTime NOTIFY minTimeChanged)
    Q_PROPERTY(QDateTime maxTime READ maxTime WRITE setMaxTime NOTIFY maxTimeChanged)
    Q_PROPERTY(QObject* model READ getModel WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int paintedCount READ paintedCount NOTIFY paintedCountChanged)

public:
    explicit Graph(QQuickItem* parent = Q_NULLPTR);

    const QColor& color() const { return m_color; }
    void setColor(const QColor& color);

    qreal lineWidth() const { return m_lineWidth; }
    void setLineWidth(qreal lineWidth);

    qreal minValue() const { return m_minValue; }
    void setMinValue(qreal value);

    qreal maxValue() const { return m_maxValue; }
    void setMaxValue(qreal value);

    const QDateTime& minTime() const { return m_minTime; }
    void setMinTime(const QDateTime& t);

    const QDateTime& maxTime() const { return m_maxTime; }
    void setMaxTime(const QDateTime& t);

    QAbstractItemModel* getModel() const { return m_model; }
    void setModel(QObject* model);

    int paintedCount() const { return m_paintedCount; }

protected:
    void paint(QPainter* aPainter) Q_DECL_OVERRIDE;

private:
    static bool lineVisible(qreal x1, qreal y1, qreal x2, qreal y2, qreal w, qreal h);

private slots:
    void onModelDestroyed();

signals:
    void colorChanged();
    void lineWidthChanged();
    void minValueChanged();
    void maxValueChanged();
    void minTimeChanged();
    void maxTimeChanged();
    void modelChanged();
    void paintedCountChanged();

private:
    QColor m_color;
    qreal m_lineWidth;
    qreal m_minValue;
    qreal m_maxValue;
    QDateTime m_minTime;
    QDateTime m_maxTime;
    int m_timestampRole;
    int m_valueRole;
    QAbstractItemModel* m_model;
    int m_paintedCount;
};

#endif // GRAPH_H
