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

#include "valuegridmodel.h"
#include "debuglog.h"

#include <qmath.h>

class ValueGridModel::Grid {
public:
    enum Role {
        ValueRole = Qt::UserRole,
        TextRole,
        CoordinateRole
    };

    Grid() : value(0.0), coord(0.0) {}
    Grid(const Grid& g) : value(g.value), coord(g.coord), text(g.text) {}
    Grid(qreal v, qreal c, qreal s) : value(v), coord(c), text(formatValue(v, s)) {}

    static QString formatValue(qreal v, qreal s);
    Grid& operator=(const Grid& g) { value = g.value; coord = g.coord; text = g.text; return *this; }
    bool operator==(const Grid& g) const { return value == g.value && coord == g.coord && text == g.text; }

public:
    qreal value;
    qreal coord;
    QString text;
};

QString ValueGridModel::Grid::formatValue(qreal v, qreal s)
{
    static const QString zero("0");
    static const QString sep1(".");
    static const QString sep2(",");
    int precision = 2;
    if (v == 0.0) {
        return zero;
    } else if (s < 1) {
        precision = qMax((int)ceil(-log10(s)) + 1, precision);
    } else if (ceil(log10(qAbs(v))) > 6) {
        return QString::number(v, 'g', 5);
    }
    // Apply 'f' format
    QString text(QString::number(v, 'f', precision));
    // Strip trailing zeros
    while (text.endsWith(zero)) {
        text = text.left(text.length() - 1);
    }
    if (text.endsWith(sep1) || text.endsWith(sep2)) {
        text = text.left(text.length() - 1);
    }
    return text;
}

ValueGridModel::ValueGridModel(QObject* parent) :
    QAbstractListModel(parent),
    m_size(1.0),
    m_minValue(0.0),
    m_maxValue(1.0),
    m_maxCount(10),
    m_fixedGrids(false)
{
}

ValueGridModel::~ValueGridModel()
{
}

void ValueGridModel::setSize(qreal size)
{
    if (m_size != size) {
        m_size = size;
        DBG(size);
        updateGrids();
        emit sizeChanged();
    }
}

void ValueGridModel::setMinValue(qreal value)
{
    if (m_minValue != value) {
        m_minValue = value;
        DBG(value);
        updateGrids();
        emit minValueChanged();
    }
}

void ValueGridModel::setMaxValue(qreal value)
{
    if (m_maxValue != value) {
        m_maxValue = value;
        DBG(value);
        updateGrids();
        emit maxValueChanged();
    }
}

void ValueGridModel::setMaxCount(int count)
{
    if (m_maxCount != count) {
        m_maxCount = count;
        DBG(count);
        updateGrids();
        maxCountChanged();
    }
}

void ValueGridModel::setFixedGrids(bool fixed)
{
    if (m_fixedGrids != fixed) {
        m_fixedGrids = fixed;
        DBG(fixed);
        updateGrids();
        fixedGridsChanged();
    }
}

bool ValueGridModel::makeGrids(QVector<Grid>* grids, qreal step)
{
    grids->resize(0);
    const qreal span = m_maxValue - m_minValue;
    for (int i = ceil(m_minValue/step); step * i < m_maxValue; i++) {
        if (grids->count() >= m_maxCount) {
            DBG("step" << step << "is too small");
            return false;
        }
        const qreal value = step * i;
        const qreal coord = m_size * (value - m_minValue) / span;
        const Grid grid(value, coord, step);
        grids->append(grid);
        DBG(grids->count() << ":" << grid.value << grid.coord << grid.text);
    }
    return true;
}

void ValueGridModel::updateGrids()
{
    QVector<Grid> grids;

    if (m_size > 0 && m_maxValue > m_minValue && m_maxCount > 0) {
        const qreal span = m_maxValue - m_minValue;
        if (m_fixedGrids) {
            const qreal step = span/(m_maxCount + 1);
            DBG("grid" << m_minValue << ".." << m_maxValue);
            for (int i = 0; i < m_maxCount; i++) {
                const qreal offset = (i + 1) * step;
                const qreal coord = m_size * offset / span;
                const Grid grid(m_minValue + offset, coord, step);
                grids.append(grid);
                DBG(grids.count() << ":" << grid.value << grid.coord << grid.text);
            }
        } else {
            const qreal minStep = span/m_maxCount;
            const int log = ceil(log10(minStep));
            const qreal roundedStep = exp10(log);
            DBG("grid" << m_minValue << ".." << m_maxValue << log << roundedStep);
            /* First try to make more grids */
            if (!makeGrids(&grids, roundedStep/10) &&
                !makeGrids(&grids, roundedStep/5)  &&
                !makeGrids(&grids, roundedStep/2)) {
                makeGrids(&grids, roundedStep);
            }
        }
    }

    if (m_grids != grids) {
        beginResetModel();
        m_grids = grids;
        endResetModel();
    }
}

/* QAbstractItemModel */

QHash<int,QByteArray> ValueGridModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles.insert(Grid::ValueRole, "value");
    roles.insert(Grid::TextRole, "text");
    roles.insert(Grid::CoordinateRole, "coordinate");
    return roles;
}

int ValueGridModel::rowCount(const QModelIndex& parent) const
{
    return m_grids.count();
}

QVariant ValueGridModel::data(const QModelIndex& idx, int role) const
{
    const int row = idx.row();
    if (row >= 0 && row < m_grids.count()) {
        const Grid& grid = m_grids.at(row);
        switch ((Grid::Role)role) {
        case Grid::ValueRole: return grid.value;
        case Grid::TextRole: return grid.text;
        case Grid::CoordinateRole: return grid.coord;
        }
    }
    return QVariant();
}
