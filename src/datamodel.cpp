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

#include "database.h"
#include "datamodel.h"
#include "debuglog.h"

enum DataRole {
    KeyRole = Qt::UserRole,
    TimestampRole,
    TimestampUtcRole,
    ValueRole,
    AnnotationRole
};

#define ROLE_TIMESTAMP_UTC_ROLE "timestampUTC"

namespace {
    const QString KEY(ROLE_KEY_ROLE);
    const QString TIMESTAMP(ROLE_TIMESTAMP_ROLE);
    const QString VALUE(ROLE_VALUE_ROLE);
    const QString ANNOTATION(ROLE_ANNOTATION_ROLE);
}

// UTC times are signiticantly faster to compare.
// Local times are getting converted to UTC every time
// QDateTimePrivate::toMSecsSinceEpoch() is called.

class DataModel::Data {
public:
    Data(const QVariantMap& data) :
        key(data.value(KEY).toString()),
        value(data.value(VALUE).toString()),
        annotation(data.value(ANNOTATION).toString()),
        timestamp(data.value(TIMESTAMP).toDateTime()),
        timestampUtc(timestamp.toUTC()) {}
public:
    const QString key;
    QString value;
    QString annotation;
    QDateTime timestamp;
    QDateTime timestampUtc;
};

DataModel::DataModel(QObject* parent) :
    QAbstractListModel(parent),
    m_minValue(0.0),
    m_maxValue(0.0)
{
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), SIGNAL(countChanged()));
}

DataModel::~DataModel()
{
    qDeleteAll(m_data);
}

void DataModel::setRawData(QVariantList list)
{
    if (m_rawData != list) {
        const QDateTime prevMinTime(m_minTime);
        const QDateTime prevMaxTime(m_maxTime);
        const qreal prevMinValue(m_minValue);
        const qreal prevMaxValue(m_maxValue);

        beginResetModel();
        m_rawData = list;

        qDeleteAll(m_data);
        m_data.resize(0);
        m_minTime = m_maxTime = QDateTime();
        m_minValue = m_maxValue = 0.0;
        const int n = list.count();
        DBG(n << "items");
        if (n > 0) {

            #define FIRST_TIME 0x01
            #define FIRST_VALUE 0x02

            m_data.reserve(n);
            int first = (FIRST_TIME | FIRST_VALUE);
            for (int i = 0; i < n; i++) {
                Data* data = new Data(list.at(i).toMap());
                m_data.append(data);

                if (data->timestampUtc.isValid()) {
                    if (first & FIRST_TIME) {
                        first &= ~FIRST_TIME;
                        m_minTime = m_maxTime = data->timestampUtc;
                    } else if (m_minTime > data->timestampUtc) {
                        m_minTime = data->timestampUtc;
                    } else if (m_maxTime < data->timestampUtc) {
                        m_maxTime = data->timestampUtc;
                    }
                }

                bool ok;
                const qreal value = data->value.toFloat(&ok);
                if (ok) {
                    if (first & FIRST_VALUE) {
                        first &= ~FIRST_VALUE;
                        m_minValue = m_maxValue = value;
                    } else if (m_minValue > value) {
                        m_minValue = value;
                    } else if (m_maxValue < value) {
                        m_maxValue = value;
                    }
                }
            }
            // Convert back to local
            m_minTime = m_minTime.toLocalTime();
            m_maxTime = m_maxTime.toLocalTime();
        }
        endResetModel();

        DBG("Start:" << m_minTime);
        DBG("End:" << m_maxTime);
        DBG("Min:" << m_minValue);
        DBG("Max:" << m_maxValue);

        emit rawDataChanged();
        if (m_minTime != prevMinTime) {
            emit minTimeChanged();
        }
        if (m_maxTime != prevMaxTime) {
            emit maxTimeChanged();
        }
        if (m_minValue != prevMinValue) {
            emit minValueChanged();
        }
        if (m_maxValue != prevMaxValue) {
            emit maxValueChanged();
        }
    }
}

void DataModel::updateRanges()
{
    const QDateTime prevMinTime(m_minTime);
    const QDateTime prevMaxTime(m_maxTime);
    const qreal prevMinValue(m_minValue);
    const qreal prevMaxValue(m_maxValue);

    m_minTime = m_maxTime = QDateTime();
    m_minValue = m_maxValue = 0.0;

    #define FIRST_TIME 0x01
    #define FIRST_VALUE 0x02

    int first = (FIRST_TIME | FIRST_VALUE);
    const int n = m_data.count();
    for (int i = 0; i < n; i++) {
        const Data* data = m_data.at(i);

        if (data->timestampUtc.isValid()) {
            if (first & FIRST_TIME) {
                first &= ~FIRST_TIME;
                m_minTime = m_maxTime = data->timestampUtc;
            } else if (m_minTime > data->timestampUtc) {
                m_minTime = data->timestampUtc;
            } else if (m_maxTime < data->timestampUtc) {
                m_maxTime = data->timestampUtc;
            }
        }

        bool ok;
        const qreal value = data->value.toFloat(&ok);
        if (ok) {
            if (first & FIRST_VALUE) {
                first &= ~FIRST_VALUE;
                m_minValue = m_maxValue = value;
            } else if (m_minValue > value) {
                m_minValue = value;
            } else if (m_maxValue < value) {
                m_maxValue = value;
            }
        }
    }
    // Convert back to local
    m_minTime = m_minTime.toLocalTime();
    m_maxTime = m_maxTime.toLocalTime();

    DBG("Start:" << m_minTime);
    DBG("End:" << m_maxTime);
    DBG("Min:" << m_minValue);
    DBG("Max:" << m_maxValue);

    emit rawDataChanged();
    if (m_minTime != prevMinTime) {
        emit minTimeChanged();
    }
    if (m_maxTime != prevMaxTime) {
        emit maxTimeChanged();
    }
    if (m_minValue != prevMinValue) {
        emit minValueChanged();
    }
    if (m_maxValue != prevMaxValue) {
        emit maxValueChanged();
    }
}

void DataModel::deleteRow(int row)
{
    if (row >= 0 && row < m_data.count()) {
        DBG("deleting row" << row);
        beginRemoveRows(QModelIndex(), row, row);
        Data* entry = m_data.at(row);
        deleteRowStorage(entry->key);
        m_data.remove(row);
        delete entry;
        endRemoveRows();
        updateRanges();
    }
}

void DataModel::deleteRowStorage(QString key)
{
    // Placeholder for DataTableModel
}

void DataModel::updateRow(int row, QString value, QString annotation, QString timestamp)
{
    if (row >= 0 && row < m_data.count()) {
        Data* data = m_data.at(row);
        QVector<int> roles;

        DBG("updating row" << row);
        if (data->value != value) {
            DBG(VALUE << "=" << value);
            data->value = value;
            roles.append(ValueRole);
        }

        if (data->annotation != annotation) {
            DBG(ANNOTATION << "=" << annotation);
            data->annotation = annotation;
            roles.append(AnnotationRole);
        }
        const QString currentTimestamp(Database::toString(data->timestamp));
        if (currentTimestamp != timestamp) {
            DBG(TIMESTAMP << "=" << timestamp);
            data->timestamp = Database::toDateTime(timestamp);
            data->timestampUtc = data->timestamp.toUTC();
            roles.append(TimestampRole);
        }

        if (!roles.isEmpty() &&
            updateRowStorage(data->key, value, annotation, timestamp)) {
            const QModelIndex idx(index(row));
            emit dataChanged(idx, idx, roles);
            updateRanges();
        }
    }
}

bool DataModel::updateRowStorage(QString key, QString value, QString annotation, QString timestamp)
{
    // Placeholder for DataTableModel
    return true;
}

/* QAbstractItemModel */

QHash<int,QByteArray> DataModel::roleNames() const
{
    QHash<int,QByteArray> roles;

    roles.insert(KeyRole, ROLE_KEY_ROLE);
    roles.insert(TimestampRole, ROLE_TIMESTAMP_ROLE);
    roles.insert(TimestampUtcRole, ROLE_TIMESTAMP_UTC_ROLE);
    roles.insert(ValueRole, ROLE_VALUE_ROLE);
    roles.insert(AnnotationRole, ROLE_ANNOTATION_ROLE);
    return roles;
}

int DataModel::rowCount(const QModelIndex& parent) const
{
    return m_data.count();
}

QVariant DataModel::data(const QModelIndex& idx, int role) const
{
    const int row = idx.row();
    if (row >= 0 && row < m_data.count()) {
        const Data* data = m_data.at(row);
        switch (role) {
        case KeyRole: return data->key;
        case TimestampRole: return data->timestamp;
        case TimestampUtcRole: return data->timestampUtc;
        case ValueRole: return data->value;
        case AnnotationRole: return data->annotation;
        }
    }
    return QVariant();
}
