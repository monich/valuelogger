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

#include "colormodel.h"
#include "debuglog.h"

enum ColorModelRole {
    ColorRole = Qt::UserRole,
    ItemTypeRole
};

ColorModel::ColorModel(QObject* parent) :
    QAbstractListModel(parent),
    m_dragPos(-1),
    m_dragStartPos(-1),
    m_addColor(0, 0, 0, 0)
{
}

QHash<int,QByteArray> ColorModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles.insert(ColorRole, "color");
    roles.insert(ItemTypeRole, "itemType");
    return roles;
}

int ColorModel::rowCount(const QModelIndex&) const
{
    return m_colors.count() + 1;
}

QVariant ColorModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();
    if (row >= 0) {
        const int n = m_colors.count();
        if (row <= n) {
            switch ((ColorModelRole)role) {
            case ColorRole:
                if (m_dragPos >= 0) {
                    if (row == m_dragPos) {
                        // This includes the item dragged outside
                        return m_colors.at(m_dragStartPos);
                    } else {
                        if (m_dragPos == n && row == (n - 1)) {
                            // When item is dragged outside, it becomes
                            // the last one and shifts the "add" item.
                            return m_addColor;
                        } else if (row >= m_dragStartPos && row < m_dragPos) {
                            return m_colors.at(row + 1);
                        } else if (row > m_dragPos && row <= m_dragStartPos) {
                            return m_colors.at(row - 1);
                        }
                    }
                }
                return (row == n) ? m_addColor : m_colors.at(row);
            case ItemTypeRole:
                if (m_dragPos == n) {
                    // Item is dragged outside
                    return (row == n) ? TrashedItem :
                        (row == (n - 1)) ? AddItem : ColorItem;
                } else {
                    return (row == n) ? AddItem : ColorItem;
                }
            }
        }
    }
    return QVariant();
}

QStringList ColorModel::getColors() const
{
    const int n = m_colors.count();
    QStringList colors;
    colors.reserve(n);
    for (int i = 0; i < n; i++) {
        colors.append(m_colors.at(i).name());
    }
    return colors;
}

void ColorModel::setColors(QStringList colors)
{
    const int n = colors.count();
    bool changed = m_colors.count() != n;
    QList<QColor> newColors;
    newColors.reserve(n);
    for (int i = 0; i < n; i++) {
        const QColor c(colors.at(i));
        newColors.append(c);
        if (!changed && m_colors.at(i) != c) {
            changed = true;
        }
    }
    if (changed) {
        beginResetModel();
        m_colors = newColors;
        if (m_dragPos >= 0) {
            m_dragPos = -1;
            m_dragStartPos = -1;
            Q_EMIT dragPosChanged();
        }
        endResetModel();
        Q_EMIT colorsChanged();
    }
}

void ColorModel::addColor(QColor color)
{
    if (color.isValid()) {
        const int n = m_colors.count();
        beginInsertRows(QModelIndex(), n, n);
        m_colors.append(color);
        endInsertRows();
        Q_EMIT colorsChanged();
    }
}

int ColorModel::getDragPos() const
{
    return m_dragPos;
}

void ColorModel::setDragPos(int pos)
{
    if (pos < 0) {
        if (m_dragPos >= 0) {
            // The drag is finished
            if (m_dragPos != m_dragStartPos) {
                if (m_dragPos == m_colors.count()) {
                    DBG("trashed" << m_dragStartPos);
                    beginRemoveRows(QModelIndex(), m_dragPos, m_dragPos);
                    m_colors.removeAt(m_dragStartPos);
                    m_dragPos = m_dragStartPos = -1;
                    endRemoveRows();
                } else {
                    DBG("dragged" << m_dragStartPos << "=>" << m_dragPos);
                    m_colors.move(m_dragStartPos, m_dragPos);
                    m_dragPos = m_dragStartPos = -1;
                }
                Q_EMIT colorsChanged();
            } else {
                DBG("drag cancelled");
                m_dragPos = m_dragStartPos = -1;
            }
            Q_EMIT dragPosChanged();
        }
    } else {
        const int n = m_colors.count();
        if (pos >= n) pos = n;
        if (pos != m_dragPos) {
            if (m_dragPos >= 0) {
                const QModelIndex parent;
                const bool wasTrashed = (m_dragPos == n);
                const bool isTrashed = (pos == n);
                DBG(pos << (isTrashed ? "(outside)" : ""));
                const int dest = (pos > m_dragPos) ? (pos + 1) : pos;
                beginMoveRows(QModelIndex(), m_dragPos, m_dragPos, QModelIndex(), dest);
                m_dragPos = pos;
                endMoveRows();
                if (wasTrashed || isTrashed) {
                    const QVector<int> roles(1, ItemTypeRole);
                    const QModelIndex modelIndex(index(m_dragPos));
                    Q_EMIT dataChanged(modelIndex, modelIndex, roles);
                }
                Q_EMIT dragPosChanged();
            } else if (pos < n /* Must be within bounds */) {
                // Drag is starting
                DBG("dragging" << pos);
                m_dragPos = m_dragStartPos = pos;
                Q_EMIT dragPosChanged();
            }
        }
    }
}
