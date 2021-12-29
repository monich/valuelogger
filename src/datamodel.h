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

#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QDateTime>
#include <QVariantList>
#include <QAbstractListModel>

#define ROLE_KEY_ROLE           "key"
#define ROLE_TIMESTAMP_ROLE     "timestamp"
#define ROLE_VALUE_ROLE         "value"
#define ROLE_ANNOTATION_ROLE    "annotation"

class DataModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QVariantList rawData READ getRawdata WRITE setRawData NOTIFY rawDataChanged)
    Q_PROPERTY(QDateTime minTime READ getMinTime NOTIFY minTimeChanged)
    Q_PROPERTY(QDateTime maxTime READ getMaxTime NOTIFY maxTimeChanged)
    Q_PROPERTY(qreal minValue READ getMinValue NOTIFY minValueChanged)
    Q_PROPERTY(qreal maxValue READ getMaxValue NOTIFY maxValueChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    explicit DataModel(QObject* parent = Q_NULLPTR);
    ~DataModel();

    QVariantList getRawdata() const { return m_rawData; }
    void setRawData(QVariantList data);

    const QDateTime& getMinTime() const { return m_minTime; }
    const QDateTime& getMaxTime() const { return m_maxTime; }
    qreal getMinValue() const { return m_minValue; }
    qreal getMaxValue() const { return m_maxValue; }

    Q_INVOKABLE void deleteRow(int row);
    Q_INVOKABLE void updateRow(int row, QString value, QString annotation, QString timestamp);

    /* QAbstractItemModel */
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& idx, int role) const Q_DECL_OVERRIDE;

protected:
    virtual void deleteRowStorage(QString key);
    virtual bool updateRowStorage(QString key, QString value, QString annotation, QString timestamp);

private:
    void updateRanges();

signals:
    void rawDataChanged();
    void minTimeChanged();
    void maxTimeChanged();
    void minValueChanged();
    void maxValueChanged();
    void countChanged();

private:
    class Data;
    QVariantList m_rawData;
    QVector<Data*> m_data;
    QDateTime m_minTime;
    QDateTime m_maxTime;
    qreal m_minValue;
    qreal m_maxValue;
};

#endif // DATAMODEL_H
