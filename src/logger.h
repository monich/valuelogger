/*
Copyright (c) 2014-2015 kimmoli kimmo.lindholm@gmail.com @likimmo
Copyright (c) 2021 Slava Monich

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QColor>
#include <QString>
#include <QVariantList>
#include <QSqlDatabase>

class QQmlEngine;
class QJSEngine;

class Logger : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ getVersion CONSTANT)

public:
    explicit Logger(QObject* parent = Q_NULLPTR);
    ~Logger();

    QString getVersion();

    Q_INVOKABLE QString addParameterEntry(QString key, QString parameterName, QString parameterDescription, bool visualize, QColor plotColor, QString pairedtable);
    Q_INVOKABLE void deleteParameterEntry(QString parameterName, QString datatable);
    Q_INVOKABLE QVariantList readParameters();
    Q_INVOKABLE QVariantList readData(QString table);
    Q_INVOKABLE QString addData(QString table, QString key, QString value, QString annotation, QString timestamp);
    Q_INVOKABLE void deleteData(QString table, QString key);
    Q_INVOKABLE void setPairedTable(QString datatable, QString pairedtable);
    Q_INVOKABLE QString exportToCSV();

    /* Callback for qmlRegisterSingletonType<Logger> */
    static QObject* createSingleton(QQmlEngine* engine, QJSEngine* js);

private:
    static QString generateHash(QString sometext);

    void dropDataTable(QString table);
    void closeDatabase();
    void createParameterTable();
    void createDataTable(QString table);

private:
    static const QString DB_NAME;
    QSqlDatabase db;
};

#endif // LOGGER_H
