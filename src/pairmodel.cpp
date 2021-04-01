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

#include "pairmodel.h"
#include "logger.h"

#include <QRegExp>

PairModel::PairModel(QObject* parent) :
    QSortFilterProxyModel(parent)
{
    setFilterRole(Logger::dataTableRole());
}

/* This is easier than to build a regex for inverse matching */
bool PairModel::filterAcceptsRow(int row, const QModelIndex& parent) const
{
    return m_ignoreDataTable.isEmpty() ||
        !QSortFilterProxyModel::filterAcceptsRow(row, parent);
}

void PairModel::setIgnoreDataTable(QString table)
{
    if (m_ignoreDataTable != table) {
        beginResetModel();
        m_ignoreDataTable = table;
        endResetModel();
        emit ignoreDataTableChanged();
        setFilterRegExp(table.isEmpty() ?  QRegExp() :
            QRegExp("^" + table + "$")); /* inverted by filterAcceptsRow */
    }
}
