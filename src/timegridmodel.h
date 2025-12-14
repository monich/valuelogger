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

#ifndef TIMEGRIDMODEL_H
#define TIMEGRIDMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QVector>

class TimeGridModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(qreal size READ getSize WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(int minCount READ getMinCount WRITE setMinCount NOTIFY minCountChanged)
    Q_PROPERTY(int maxCount READ getMaxCount WRITE setMaxCount NOTIFY maxCountChanged)
    Q_PROPERTY(QDateTime timeOrigin READ getTimeOrigin WRITE setTimeOrigin NOTIFY timeOriginChanged)
    Q_PROPERTY(QDateTime timeStart READ getTimeStart WRITE setTimeStart NOTIFY timeStartChanged)
    Q_PROPERTY(QDateTime timeEnd READ getTimeEnd WRITE setTimeEnd NOTIFY timeEndChanged)
    Q_PROPERTY(bool fixedGrids READ getFixedGrids WRITE setFixedGrids NOTIFY fixedGridsChanged)
    struct Step;

public:
    class Grid;
    explicit TimeGridModel(QObject* parent = Q_NULLPTR);
    ~TimeGridModel();

    qreal getSize() const { return m_size; }
    int getMinCount() const { return m_minCount; }
    int getMaxCount() const { return m_maxCount; }
    QDateTime getTimeOrigin() const { return m_timeOrigin; }
    QDateTime getTimeStart() const { return m_timeStart; }
    QDateTime getTimeEnd() const { return m_timeEnd; }
    bool getFixedGrids() const { return m_fixedGrids; }

    void setSize(qreal size);
    void setMinCount(int count);
    void setMaxCount(int count);
    void setTimeOrigin(QDateTime t);
    void setTimeStart(QDateTime t);
    void setTimeEnd(QDateTime t);
    void setFixedGrids(bool fixed);

    /* QAbstractItemModel */
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& idx, int role) const Q_DECL_OVERRIDE;

private:
    bool makeGrids(QVector<Grid>* grids, const Step* step, int n) const;
    bool makeGrids(QVector<Grid>* grids, const Step* step) const;
    void updateGrids();

signals:
    void sizeChanged();
    void minCountChanged();
    void maxCountChanged();
    void timeOriginChanged();
    void timeStartChanged();
    void timeEndChanged();
    void fixedGridsChanged();

private:
    QVector<Grid> m_grids;
    qreal m_size;
    int m_minCount;
    int m_maxCount;
    QDateTime m_timeOrigin;
    QDateTime m_timeStart;
    QDateTime m_timeEnd;
    bool m_fixedGrids;
};

#endif // TIMEGRIDMODEL_H
