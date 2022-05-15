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
#include "debuglog.h"

#include <QDir>
#include <QTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QWeakPointer>

#define PARAMETERS_TABLE    "parameters"

namespace {
    const QString DB_NAME("QSQLITE");

    const QString PARAMETER(PARAMETER_COL);
    const QString NAME(NAME_ROLE);
    const QString DESCRIPTION(DESCRIPTION_ROLE);
    const QString VISUALIZE(VISUALIZE_ROLE);
    const QString PLOTCOLOR(PLOTCOLOR_ROLE);
    const QString DATATABLE(DATATABLE_ROLE);
    const QString PAIREDTABLE(PAIREDTABLE_ROLE);

    const QString KEY(DATA_KEY_COL);
    const QString TIMESTAMP(DATA_TIMESTAMP_COL);
    const QString VALUE(DATA_VALUE_COL);
    const QString ANNOTATION(DATA_ANNOTATION_COL);

    const QString FORMAT_DATE_TIME("yyyy-MM-dd hh:mm:ss");
    const QString FORMAT_DATE("yyyy-MM-dd");
}

class Database::Private : public QObject
{
    Q_OBJECT

public:
    Private();
    ~Private();

    static QString generateHash(QString text);
    void createParameterTable();
    void createDataTable(QString table);
    void dropDataTable(QString table);

signals:
    void dataChanged(QString table);

public:
    QSqlDatabase db;
};

Database::Private::Private() :
    db(QSqlDatabase::addDatabase(DB_NAME))
{
    /* Open the harbour-valuelogger (not harbour-valuelogger2) database */
    const QString shareDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    QDir dbdir(shareDir + "/harbour-valuelogger/harbour-valuelogger");
    if (!dbdir.exists()) {
        dbdir.mkpath(dbdir.path());
    }

    const QString dbpath(dbdir.absoluteFilePath("valueLoggerDb.sqlite"));
    DBG(dbpath);

    db.setDatabaseName(dbpath);
    if (db.open()) {
        DBG("Open Success");
    } else {
        WARN("Open error" << db.lastError().text());
    }

    createParameterTable();
}

Database::Private::~Private()
{
    DBG("Closing db");
    db.removeDatabase(DB_NAME);
    db.close();
}

/*
 * Generates md5 of some data
 */

QString Database::Private::generateHash(QString text)
{
    const int rnd = qrand();
    const QString tmp(QString("%1 %2 %3").arg(text).arg(rnd).arg(QTime::currentTime().hour()));
    return QString(QCryptographicHash::hash((tmp.toUtf8()),QCryptographicHash::Md5).toHex());
}

/*
 * Parameter table collects parameters to which under data is being logged
 */

void Database::Private::createParameterTable()
{
    QSqlQuery query(db);

    if (query.exec("CREATE TABLE IF NOT EXISTS " PARAMETERS_TABLE " ("
        PARAMETER_COL " TEXT, " DESCRIPTION_COL " TEXT, "
        VISUALIZE_COL " INTEGER, " PLOTCOLOR_COL " TEXT, "
        DATATABLE_COL " TEXT PRIMARY KEY, " PAIREDTABLE_COL " TEXT)")) {
        DBG("parameter table created");
    } else {
        WARN("parameter table not created :" << query.lastError());
    }
}

/*
 * Create table for data storage, each parameter has its own table
 * table name is prefixed with _ to allow number-starting tables
 */

void Database::Private::createDataTable(QString table)
{
    const QString datatable("_" + table);
    QSqlQuery query(db);

    if (query.exec("CREATE TABLE IF NOT EXISTS " + datatable + " ("
        DATA_KEY_COL " TEXT PRIMARY KEY, " DATA_TIMESTAMP_COL " TEXT, "
        DATA_VALUE_COL " TEXT, " DATA_ANNOTATION_COL " TEXT)")) {
        DBG("datatable created" << datatable);
    } else {
        WARN("datatable not created" << datatable << ":" << query.lastError());
    }
}

/*
 * When parameter is deleted, we need also to drop the datatable connected to it
 */

void Database::Private::dropDataTable(QString table)
{
    const QString datatable("_" + table);
    QSqlQuery query(db);

    if (query.exec("DROP TABLE IF EXISTS " + datatable)) {
        DBG("datatable dropped" << datatable);
    } else {
        WARN("datatable not dropped" << datatable << ":" << query.lastError());
    }
}

/*
 * Add created parameter entry to parameter table
 *
 * datatable name is md5 of timestamp + random number
 */

bool Database::insertOrReplace(QString datatable, QString name, QString description, bool visualize, QString plotColor, QString pairedtable)
{
    QSqlQuery addColQuery("ALTER TABLE " PARAMETERS_TABLE " ADD COLUMN " PAIREDTABLE_COL " TEXT", p->db);

    if (addColQuery.exec()) {
        DBG("column pairedtable added succesfully");
    }

    QSqlQuery query("INSERT OR REPLACE INTO " PARAMETERS_TABLE " ("
        PARAMETER_COL "," DESCRIPTION_COL "," VISUALIZE_COL ","
        PLOTCOLOR_COL "," DATATABLE_COL "," PAIREDTABLE_COL ") "
        "VALUES (?,?,?,?,?,?)", p->db);
    query.addBindValue(name);
    query.addBindValue(description);
    query.addBindValue(visualize ? 1:0 ); // store bool as integer
    query.addBindValue(plotColor);
    query.addBindValue(datatable);
    query.addBindValue(pairedtable);

    if (query.exec()) {
        p->createDataTable(datatable);
        return true;
    } else {
        WARN("insert or replace failed" << name << ":" << query.lastError());
        return false;
    }
}

/*
 * Read all parameters (to be shown on mainpage listview
 */

QVariantList Database::readParameters()
{
    QSqlQuery query("SELECT * FROM " PARAMETERS_TABLE " ORDER BY " PARAMETER_COL " ASC", p->db);
    QVariantList list;

    if (query.exec()) {
        while (query.next()) {
            const QSqlRecord record(query.record());
            QVariantMap map;

            map.insert(NAME, record.value(PARAMETER));
            map.insert(DESCRIPTION, record.value(DESCRIPTION));
            map.insert(VISUALIZE, record.value(VISUALIZE).toInt() > 0);
            map.insert(PLOTCOLOR, record.value(PLOTCOLOR));
            map.insert(DATATABLE, record.value(DATATABLE));
            map.insert(PAIREDTABLE, record.value(PAIREDTABLE));

            list.append(map);
        }
    } else {
        WARN("readParameters failed" << query.lastError());
    }

    return list;
}

/*
 * Get all data of one parameter, to raw data show page or for plotting
 */

QVariantList Database::readData(QString table)
{
    const QString datatable("_" + table);
    QSqlQuery query(p->db);
    QVariantList list;

    if (query.exec("SELECT * FROM " + datatable + " ORDER BY timestamp ASC")) {
        while (query.next()) {
            const QSqlRecord record(query.record());
            QVariantMap map;

            map.insert(KEY, record.value(KEY));
            map.insert(TIMESTAMP, toDateTime(record.value(TIMESTAMP)));
            map.insert(VALUE, record.value(VALUE));
            map.insert(ANNOTATION, record.value(ANNOTATION));

            list.append(map);
        }
    } else {
        WARN("readData" << datatable << "failed" << query.lastError());
    }

    return list;
}

/*
 * Add new data entry to a parameter
 * key = "" to generate new
 */

QString Database::addData(QString table, QString key, QString value, QString annotation, QString timestamp)
{
    const QString datatable("_" + table);
    DBG("Adding" << value << "(" << timestamp << ")" << annotation << "to" << datatable);

    QSqlQuery addColQuery(p->db);

    if (addColQuery.exec("ALTER TABLE " + datatable + " ADD COLUMN annotation TEXT")) {
        DBG("column annotation added succesfully");
    }

    QSqlQuery query("INSERT OR REPLACE INTO " + datatable + " (key,timestamp,value,annotation) VALUES (?,?,?,?)", p->db);
    const QString objHash(key.isEmpty() ? Private::generateHash(value) : key);
    query.addBindValue(objHash);
    query.addBindValue(timestamp);
    query.addBindValue(value);
    query.addBindValue(annotation);

    if (query.exec()) {
        DBG("data" << (key.isEmpty() ? "added" : "edited") << timestamp << "=" << value << "+" << annotation);
        emit p->dataChanged(table);
        return objHash;
    } else {
        WARN("failed" << timestamp << "=" << value << ":" << query.lastError());
        return QString();
    }
}

/*
 * Adds new parameter, returns the datatable
 */

QString Database::addParameter(QString name, QString description, bool visualize, QString plotcolor)
{
    const QString datatable(Private::generateHash(name));
    if (insertOrReplace(datatable, name, description, visualize, plotcolor, QString())) {
        return datatable;
    } else {
        return QString();
    }
}

/*
 * Delete one parameter, deletes also associated datatable
 */

void Database::deleteParameter(QString datatable)
{
    QSqlQuery query = QSqlQuery("DELETE FROM " PARAMETERS_TABLE " WHERE " DATATABLE_COL " = ?", p->db);
    query.addBindValue(datatable);

    if (!query.exec()) {
        WARN("deleteParameterEntry failed");
    }

    p->dropDataTable(datatable);
}

/*
 * Delete one data entry of a parameter
 */

void Database::deleteData(QString table, QString key)
{
    const QString datatable("_" + table);

    QSqlQuery query("DELETE FROM " + datatable + " WHERE key = ?", p->db);
    query.addBindValue(key);

    if (query.exec()) {
        DBG("deleted" << key << "from" << datatable);
        emit p->dataChanged(table);
    } else {
        WARN("Failed to delete" << key << "from" << datatable << ":" << query.lastError());
    }
}

/*
 * Set paired table
 */

bool Database::setPairedTable(QString datatable, QString pairedtable)
{
    QSqlQuery pairedTableAddQuery("ALTER TABLE " PARAMETERS_TABLE " ADD COLUMN " PAIREDTABLE_COL " TEXT", p->db);

    if (pairedTableAddQuery.exec()) {
        DBG("column pairedtable added succesfully");
    }

    QSqlQuery pairQuery("UPDATE " PARAMETERS_TABLE " SET " PAIREDTABLE_COL " = ? WHERE " DATATABLE_COL " = ?", p->db);
    pairQuery.addBindValue(pairedtable);
    pairQuery.addBindValue(datatable);

    if (pairQuery.exec()) {
        DBG("paired table set" << datatable << "--" << pairedtable);
        return true;
    } else {
        WARN("paring failed" << datatable << "--" << pairedtable << ":" << pairQuery.lastError());
        return false;
    }
}

Database::Database(QObject* parent) : QObject(parent)
{
    static QWeakPointer<Private> shared;
    p = shared;
    if (p.isNull()) {
        p = QSharedPointer<Private>::create();
        shared = p;
    }
    connect(p.data(), SIGNAL(dataChanged(QString)), SIGNAL(dataChanged(QString)));
}

Database::~Database()
{
}

QDateTime Database::toDateTime(const QVariant& value)
{
    const QString s(value.toString().trimmed());
    QDateTime t = QDateTime::fromString(s, FORMAT_DATE_TIME);
    if (!t.isValid()) {
        t = QDateTime::fromString(s, FORMAT_DATE);
#if LOG_DBG
        if (t.isValid()) {
            DBG(s << "=>" << t);
        } else {
            DBG("Failed to convert" << s << "to QDateTime");
        }
#endif
    }
    return t;
}

QString Database::toString(const QDateTime& value)
{
    return value.toString(FORMAT_DATE_TIME);
}

#include "database.moc"
