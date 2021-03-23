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

#ifndef GRIDMODEL_H
#define GRIDMODEL_H

#include <QAbstractListModel>
#include <QVector>

class GridModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(qreal size READ getSize WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(qreal minValue READ getMinValue WRITE setMinValue NOTIFY minValueChanged)
    Q_PROPERTY(qreal maxValue READ getMaxValue WRITE setMaxValue NOTIFY maxValueChanged)
    Q_PROPERTY(int maxCount READ getMaxCount WRITE setMaxCount NOTIFY maxCountChanged)
    Q_PROPERTY(bool fixedGrids READ getFixedGrids WRITE setFixedGrids NOTIFY fixedGridsChanged)
    class Grid;

public:
    explicit GridModel(QObject* parent = Q_NULLPTR);
    ~GridModel();

    qreal getSize() const { return m_size; }
    qreal getMinValue() const { return m_minValue; }
    qreal getMaxValue() const { return m_maxValue; }
    int getMaxCount() const { return m_maxCount; }
    bool getFixedGrids() const { return m_fixedGrids; }

    void setSize(qreal size);
    void setMinValue(qreal value);
    void setMaxValue(qreal value);
    void setMaxCount(int count);
    void setFixedGrids(bool fixed);

    /* QAbstractItemModel */
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& idx, int role) const Q_DECL_OVERRIDE;

private:
    bool makeGrids(QVector<Grid>* grids, qreal step);
    void updateGrids();

signals:
    void sizeChanged();
    void minValueChanged();
    void maxValueChanged();
    void maxCountChanged();
    void fixedGridsChanged();

private:
    QVector<Grid> m_grids;
    qreal m_size;
    qreal m_minValue;
    qreal m_maxValue;
    int m_maxCount;
    bool m_fixedGrids;
};

#endif // GRIDMODEL_H
