/*
Copyright (c) 2021-2022 Slava Monich <slava@monich.com>

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

#include "datatablemodel.h"
#include "debuglog.h"

DataTableModel::DataTableModel(QObject* parent) :
    DataModel(parent),
    m_internalUpdate(0),
    m_db(new Database(this))
{
    connect(m_db, SIGNAL(dataChanged(QString)), SLOT(onDataChanged(QString)));
}

DataTableModel::~DataTableModel()
{
}

void DataTableModel::setDataTable(QString table)
{
    if (m_dataTable != table) {
        m_dataTable = table;
        reset();
        emit dataTableChanged();
    }
}

void DataTableModel::deleteRowStorage(QString key)
{
    if (!m_dataTable.isEmpty()) {
        m_internalUpdate++;
        m_db->deleteData(m_dataTable, key);
        m_internalUpdate--;
    }
}

bool DataTableModel::updateRowStorage(QString key, QString value, QString annotation, QString timestamp)
{
    bool ok = false;
    if (!m_dataTable.isEmpty()) {
        m_internalUpdate++;
        ok = !m_db->addData(m_dataTable, key, value, annotation, timestamp).isEmpty();
        m_internalUpdate--;
    }
    return ok;
}

void DataTableModel::reset()
{
    m_internalUpdate++;
    setRawData(m_dataTable.isEmpty() ? QVariantList() : m_db->readData(m_dataTable));
    m_internalUpdate--;
}

void DataTableModel::onDataChanged(QString table)
{
    if (!m_internalUpdate && table == m_dataTable) {
        DBG(table << "has changed, resetting the model");
        reset();
    }
}
