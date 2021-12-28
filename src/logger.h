/*
Copyright (c) 2014-2015 kimmoli <kimmo.lindholm@gmail.com> @likimmo
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

#ifndef LOGGER_H
#define LOGGER_H

#include "database.h"

#include <QColor>
#include <QString>
#include <QVariantList>
#include <QAbstractListModel>

class QQmlEngine;
class QJSEngine;

class Logger : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString version READ getVersion CONSTANT)
    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(int visualizeCount READ getVisualizeCount NOTIFY visualizeCountChanged)
    Q_PROPERTY(int defaultParameterIndex READ getDefaultParameterIndex NOTIFY defaultParameterIndexChanged)
    Q_PROPERTY(QString defaultParameterName READ getDefaultParameterName NOTIFY defaultParameterNameChanged)
    Q_PROPERTY(QString defaultParameterTable READ getDefaultParameterTable NOTIFY defaultParameterTableChanged)
    Q_PROPERTY(QColor defaultParameterColor READ getDefaultParameterColor NOTIFY defaultParameterColorChanged)

    typedef void (Logger::*SignalEmitter)();

public:
    explicit Logger(QObject* parent = Q_NULLPTR);
    ~Logger();

    static int dataTableRole();
    static QString getVersion();

    int getVisualizeCount() const { return m_visualizeCount; }
    int getDefaultParameterIndex() const { return m_defaultParameterIndex; }
    QString getDefaultParameterName() const { return m_defaultParameterName; }
    QString getDefaultParameterTable() const { return m_defaultParameterTable; }
    QColor getDefaultParameterColor() const { return m_defaultParameterColor; }

    Q_INVOKABLE QString addParameter(QString name, QString description, bool visualize, QColor plotcolor);
    Q_INVOKABLE void editParameterAt(int row, QString name, QString description, bool visualize, QColor plotcolor, QString pairedtable);
    Q_INVOKABLE void deleteParameterAt(int row);
    Q_INVOKABLE QVariantList readParameters();
    Q_INVOKABLE QVariantMap get(int row);
    Q_INVOKABLE QString addData(QString table, QString value, QString annotation, QString timestamp);
    Q_INVOKABLE QString exportToCSV();

    /* QAbstractItemModel */
    Qt::ItemFlags flags(const QModelIndex& idx) const Q_DECL_OVERRIDE;
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& idx, int role) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex& idx, const QVariant& value, int role) Q_DECL_OVERRIDE;

    /* Callback for qmlRegisterSingletonType<Logger> */
    static QObject* createSingleton(QQmlEngine* engine, QJSEngine* js);

signals:
    void rowCountChanged();
    void visualizeCountChanged();
    void defaultParameterIndexChanged();
    void defaultParameterNameChanged();
    void defaultParameterTableChanged();
    void defaultParameterColorChanged();
    void tableUpdated(QString table);

private:
    void queueSignal(uint signal);
    void emitQueuedSignals();
    int currentVisualizeCount();
    int currentDefaultParameterIndex();
    void updateVisualizeCount();
    void updateDefaultParameter();

private:
    Database m_db;
    QVariantList m_parameters;
    int m_visualizeCount;
    int m_defaultParameterIndex;
    QString m_defaultParameterName;
    QString m_defaultParameterTable;
    QColor m_defaultParameterColor;
    uint queuedSignals;
};

#endif // LOGGER_H
