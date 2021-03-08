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

#include "logger.h"
#include "debuglog.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QtSql>
#include <QTime>
#include <QColor>
#include <QFile>
#include <QLocale>

enum LoggerRole {
    NameRole = Qt::UserRole,
    DescriptionRole,
    PlotColorRole,
    DataTableRole,
    PairedTableRole,
    VisualizeRole
};

#define PARAMETERS_TABLE    "parameters"
#define PARAMETER_COL       "parameter"    /* TEXT */
#define DESCRIPTION_COL     "description"  /* TEXT */
#define VISUALIZE_COL       "visualize"    /* INTEGER */
#define PLOTCOLOR_COL       "plotcolor"    /* TEXT */
#define DATATABLE_COL       "datatable"    /* TEXT */
#define PAIREDTABLE_COL     "pairedtable"  /* TEXT */

#define NAME_ROLE           "name"
#define DESCRIPTION_ROLE    DESCRIPTION_COL
#define VISUALIZE_ROLE      VISUALIZE_COL
#define PLOTCOLOR_ROLE      PLOTCOLOR_COL
#define DATATABLE_ROLE      DATATABLE_COL
#define PAIREDTABLE_ROLE    PAIREDTABLE_COL

namespace {
    const QString DB_NAME;

    const QString PARAMETER(PARAMETER_COL);
    const QString NAME(NAME_ROLE);
    const QString DESCRIPTION(DESCRIPTION_ROLE);
    const QString VISUALIZE(VISUALIZE_ROLE);
    const QString PLOTCOLOR(PLOTCOLOR_ROLE);
    const QString DATATABLE(DATATABLE_ROLE);
    const QString PAIREDTABLE(PAIREDTABLE_ROLE);

    const QString KEY("key");
    const QString TIMESTAMP("timestamp");
    const QString VALUE("value");
    const QString ANNOTATION("annotation");
}

Logger::Logger(QObject *parent) :
    QAbstractListModel(parent),
    db(QSqlDatabase::addDatabase("QSQLITE"))
{
    /* Initialise random number generator */
    qsrand(QDateTime::currentDateTime().toTime_t());

    /* Open the SQLite database */
    QDir dbdir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
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
    parameters = readParameters();
    visualizeCount = currentVisualizeCount();
    defaultParameterIndex = currentDefaultParameterIndex();
    if (defaultParameterIndex >= 0) {
        defaultParameterName = parameters.at(defaultParameterIndex).toMap().value(NAME).toString();
    }

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(rowCountChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(rowCountChanged()));
    connect(this, SIGNAL(modelReset()), SIGNAL(rowCountChanged()));
}

/* Callback for qmlRegisterSingletonType<Logger> */
QObject* Logger::createSingleton(QQmlEngine* engine, QJSEngine* js)
{
    return new Logger();
}

QString Logger::getVersion()
{
    return APPVERSION;
}

int Logger::getVisualizeCount() const
{
    return visualizeCount;
}

int Logger::getDefaultParameterIndex() const
{
    return defaultParameterIndex;
}

QString Logger::getDefaultParameterName() const
{
    return defaultParameterName;
}

int Logger::currentVisualizeCount()
{
    const int count = parameters.count();
    int n = 0;
    for (int i = 0; i < count; i++) {
        if (parameters.at(i).toMap().value(VISUALIZE).toBool()) {
            n++;
        }
    }
    DBG(n);
    return n;
}

void Logger::updateVisualizeCount()
{
    const int n = currentVisualizeCount();
    if (visualizeCount != n) {
        DBG("changed" << visualizeCount << "=>" << n);
        visualizeCount = n;
        emit visualizeCountChanged();
    }
}

int Logger::currentDefaultParameterIndex()
{
    int index = -1;
    const int count = parameters.count();
    for (int i = 0; i < count; i++) {
        const QVariantMap par(parameters.at(i).toMap());
        if (par.value(VISUALIZE).toBool()) {
            const QString parName(par.value(NAME).toString());
            if (index < 0) {
                index = i;
            } else {
                index = -1;
                break;
            }
        }
    }
    DBG(index);
    return index;
}

void Logger::updateDefaultParameter()
{
    QString name;
    const int index = currentDefaultParameterIndex();
    if (index >= 0) {
        name = parameters.at(index).toMap().value(NAME).toString();
    }
    /* First update the state and then emit signals */
    if (defaultParameterIndex != index) {
        DBG("index changed" << defaultParameterIndex << "=>" << index);
        defaultParameterIndex = index;
        if (defaultParameterName != name) {
            DBG("name changed" << defaultParameterName << "=>" << name);
            defaultParameterName = name;
            emit defaultParameterNameChanged();
        }
        emit defaultParameterIndexChanged();
    } else if (defaultParameterName != name) {
        DBG("name changed" << defaultParameterName << "=>" << name);
        defaultParameterName = name;
        emit defaultParameterNameChanged();
    }
}

/*
 * Create table for data storage, each parameter has its own table
 * table name is prefixed with _ to allow number-starting tables
 */

void Logger::createDataTable(QString table)
{
    const QString datatable("_" + table);
    QSqlQuery query(db);

    if (query.exec("CREATE TABLE IF NOT EXISTS " + datatable +
        " (key TEXT PRIMARY KEY, timestamp TEXT, value TEXT, annotation TEXT)")) {
        DBG("datatable created" << datatable);
    } else {
        WARN("datatable not created" << datatable << ":" << query.lastError());
    }
}

/*
 * When parameter is deleted, we need also to drop the datatable connected to it
 */

void Logger::dropDataTable(QString table)
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
 * Parameter table collects parameters to which under data is being logged
 */

void Logger::createParameterTable()
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
 * Set paired table
 */
bool Logger::setPairedTable(QString datatable, QString pairedtable)
{
    QSqlQuery pairedTableAtQuery("ALTER TABLE " PARAMETERS_TABLE " ADD COLUMN " PAIREDTABLE_COL " TEXT", db);

    if (pairedTableAtQuery.exec()) {
        DBG("column pairedtable added succesfully");
    }

    QSqlQuery pairQuery("UPDATE " PARAMETERS_TABLE " SET " PAIREDTABLE_COL " = ? WHERE " DATATABLE_COL " = ?", db);
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

/*
 * Add new data entry to a parameter
 * key = "" to generate new
 */

QString Logger::addData(QString table, QString key, QString value, QString annotation, QString timestamp)
{
    const QString datatable("_" + table);
    DBG("Adding" << value << "(" << timestamp << ")" << annotation << "to" << datatable);

    QSqlQuery addColQuery(db);

    if (addColQuery.exec("ALTER TABLE " + datatable + " ADD COLUMN annotation TEXT")) {
        DBG("column annotation added succesfully");
    }

    QSqlQuery query("INSERT OR REPLACE INTO " + datatable + " (key,timestamp,value,annotation) VALUES (?,?,?,?)", db);
    const QString objHash(key.isEmpty() ? generateHash(value) : key);
    query.addBindValue(objHash);
    query.addBindValue(timestamp);
    query.addBindValue(value);
    query.addBindValue(annotation);

    if (query.exec()) {
        DBG("data" << (key.isEmpty() ? "added" : "edited") << timestamp << "=" << value << "+" << annotation);
    } else {
        WARN("failed" << timestamp << "=" << value << ":" << query.lastError());
    }
    return objHash;
}

/*
 * Get all data of one parameter, to raw data show page or for plotting
 */

QVariantList Logger::readData(QString table)
{
    const QString datatable("_" + table);
    QSqlQuery query(db);
    QVariantList list;

    if (query.exec("SELECT * FROM " + datatable + " ORDER BY timestamp ASC")) {
        while (query.next()) {
            const QSqlRecord record(query.record());
            QVariantMap map;

            map.insert(KEY, record.value(KEY));
            map.insert(TIMESTAMP, record.value(TIMESTAMP));
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
 * Get data at the specified row
 */

QVariantMap Logger::get(int row)
{
    return (row >= 0 && row < parameters.count()) ?
        parameters.at(row).toMap() : QVariantMap();
}

/*
 * Delete one data entry of a parameter
 */

void Logger::deleteData(QString table, QString key)
{
    const QString datatable("_" + table);

    QSqlQuery query("DELETE FROM " + datatable + " WHERE key = ?", db);
    query.addBindValue(key);

    if (query.exec()) {
        DBG("deleted" << key << "from" << datatable);
    } else {
        WARN("Failed to delete" << key << "from" << datatable << ":" << query.lastError());
    }
}

/*
 * Read all parameters (to be shown on mainpage listview
 */

QVariantList Logger::readParameters()
{
    QSqlQuery query("SELECT * FROM " PARAMETERS_TABLE " ORDER BY " PARAMETER_COL " ASC", db);
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
 * Generates md5 of some data
 */

QString Logger::generateHash(QString sometext)
{
    int rnd = qrand();

    QString tmp = QString("%1 %2 %3").arg(sometext).arg(rnd).arg(QTime::currentTime().hour());

    return QString(QCryptographicHash::hash((tmp.toUtf8()),QCryptographicHash::Md5).toHex());
}

/*
 * Add created parameter entry to parameter table
 *
 * datatable name is md5 of timestamp + random number
 */

bool Logger::insertOrReplace(QString datatable, QString name, QString description, bool visualize, QString plotColor, QString pairedtable)
{
    QSqlQuery addColQuery("ALTER TABLE " PARAMETERS_TABLE " ADD COLUMN " PAIREDTABLE_COL " TEXT", db);

    if (addColQuery.exec()) {
        DBG("column pairedtable added succesfully");
    }

    QSqlQuery query("INSERT OR REPLACE INTO " PARAMETERS_TABLE " ("
        PARAMETER_COL "," DESCRIPTION_COL "," VISUALIZE_COL ","
        PLOTCOLOR_COL "," DATATABLE_COL "," PAIREDTABLE_COL ") "
        "VALUES (?,?,?,?,?,?)", db);
    query.addBindValue(name);
    query.addBindValue(description);
    query.addBindValue(visualize ? 1:0 ); // store bool as integer
    query.addBindValue(plotColor);
    query.addBindValue(datatable);
    query.addBindValue(pairedtable);

    if (query.exec()) {
        createDataTable(datatable);
        return true;
    } else {
        WARN("insert or replace failed" << name << ":" << query.lastError());
        return false;
    }
}

QString Logger::addParameter(QString name, QString description, bool visualize, QColor plotcolor)
{
    DBG("Adding entry:" << name << "-" << description << "color" << plotcolor);

    const QString datatable(generateHash(name));
    const QString colorName(plotcolor.name());
    if (insertOrReplace(datatable, name, description, visualize, colorName)) {
        DBG("parameter added:" << name);

        QVariantMap newpar;
        newpar.insert(NAME, name);
        newpar.insert(DESCRIPTION, description);
        newpar.insert(VISUALIZE, visualize);
        newpar.insert(PLOTCOLOR, colorName);
        newpar.insert(DATATABLE, datatable);

        /* Find insertion position */
        const int n = rowCount();
        int row;
        for (row = 0; row < n; row++) {
            const QVariantMap par(parameters.at(row).toMap());
            if (par.value(NAME).toString() > name) {
                break;
            }
        }

        beginInsertRows(QModelIndex(), row, row);
        parameters.insert(row, newpar);
        endInsertRows();

        updateVisualizeCount();
        updateDefaultParameter();
        return datatable;
    } else {
        return QString();
    }
}

void Logger::editParameterAt(int row, QString name, QString description, bool visualize, QColor plotcolor, QString pairedtable)
{
    DBG("Editing entry at" << row << ":" << name << "-" << description << "color" << plotcolor);
    if (row >= 0 && row < parameters.count()) {
        QVariantMap par(parameters.at(row).toMap());
        QVector<int> roles;

        if (par.value(NAME).toString() != name) {
            DBG(NAME << "=" << name);
            par.insert(NAME, name);
            roles.append(NameRole);
        }

        if (par.value(DESCRIPTION).toString() != description) {
            DBG(DESCRIPTION << "=" << description);
            par.insert(DESCRIPTION, description);
            roles.append(DescriptionRole);
        }

        if (par.value(VISUALIZE).toBool() != visualize) {
            DBG(VISUALIZE << "=" << visualize);
            par.insert(VISUALIZE, visualize);
            roles.append(VisualizeRole);
        }

        const QString colorName(plotcolor.name());
        if (par.value(PLOTCOLOR).toString() != colorName) {
            DBG(PLOTCOLOR << "=" << colorName);
            par.insert(PLOTCOLOR, colorName);
            roles.append(PlotColorRole);
        }

        if (par.value(PAIREDTABLE).toString() != pairedtable) {
            DBG(PAIREDTABLE << "=" << pairedtable);
            par.insert(PAIREDTABLE, pairedtable);
            roles.append(PairedTableRole);
        }

        if (!roles.isEmpty()) {
            if (insertOrReplace(par.value(DATATABLE).toString(), name, description, visualize, colorName, pairedtable)) {
                parameters.replace(row, par);
                const QModelIndex idx(index(row));
                emit dataChanged(idx, idx, roles);

                /* Check the position */
                if (row > 0 && parameters.at(row - 1).toMap().value(NAME).toString() > name) {
                    /* Move the row down */
                    int to = row - 1;
                    while (to > 0 && parameters.at(to - 1).toMap().value(NAME).toString() > name) to--;
                    DBG("Moving" << name << row << "=>" << to);
                    beginMoveRows(QModelIndex(), row, row, QModelIndex(), to);
                    parameters.move(row, to);
                    endMoveRows();
                } else if ((row + 1) < rowCount() && parameters.at(row + 1).toMap().value(NAME).toString() < name) {
                    /* Move the row up */
                    int to = row + 1;
                    while ((to + 1) < rowCount() && parameters.at(to + 1).toMap().value(NAME).toString() < name) to++;
                    DBG("Moving" << name << row << "=>" << to);
                    beginMoveRows(QModelIndex(), row, row, QModelIndex(), to + 1);
                    parameters.move(row, to);
                    endMoveRows();
                }
            }
            if (roles.contains(VisualizeRole)) {
                updateVisualizeCount();
                updateDefaultParameter();
            } else if (roles.contains(NameRole)) {
                updateDefaultParameter();
            }
        }
    }
}

/*
 * Delete one parameter, deletes also associated datatable
 */

void Logger::deleteParameterEntry(QString datatable)
{
    QSqlQuery query = QSqlQuery("DELETE FROM " PARAMETERS_TABLE " WHERE " DATATABLE_COL " = ?", db);
    query.addBindValue(datatable);

    if (!query.exec()) {
        WARN("deleteParameterEntry failed");
    }

    dropDataTable(datatable);
}

void Logger::deleteParameterAt(int row)
{
    if (row >= 0 && row < parameters.count()) {
        const QVariantMap deleted(parameters.at(row).toMap());
        const QString dataTable(deleted.value(DATATABLE).toString());

        DBG("Deleting entry at" << row << ":" << deleted.value(NAME).toString());
        deleteParameterEntry(dataTable);

        beginRemoveRows(QModelIndex(), row, row);
        parameters.removeAt(row);
        endRemoveRows();

        for (int i = parameters.count() - 1; i >= 0; i--) {
            const QVariantMap par(parameters.at(i).toMap());
            if (par.value(PAIREDTABLE).toString() == dataTable) {
                if (setPairedTable(par.value(DATATABLE).toString(), QString())) {
                    QVector<int> roles;
                    roles.append(PairedTableRole);
                    const QModelIndex idx(index(i));
                    emit dataChanged(idx, idx, roles);
                }
            }
        }

        updateVisualizeCount();
        updateDefaultParameter();
    }
}

void Logger::closeDatabase()
{
    DBG("Closing db");
    db.removeDatabase(DB_NAME);
    db.close();
}

/*
 * Export to CSV file
 */

QString Logger::exportToCSV()
{
    DBG("Exporting");

    QLocale loc = QLocale::system(); /* Should return current locale */

    QChar separator = (loc.decimalPoint() == '.') ? ',' : ';';
    DBG("Using" << separator << "as separator");

    QString filename = QString("%1/valuelogger.csv").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    DBG("Output filename is" << filename);

    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out.setCodec("UTF-8");

    QVariantList eParameters = readParameters();
    QListIterator<QVariant> i(eParameters);

    while (i.hasNext()) {
        QVariantMap eParamData = i.next().value<QVariantMap>();

        out << eParamData[NAME].toString() << separator << eParamData[DESCRIPTION].toString() << "\n";

        QVariantList eData = readData(eParamData[DATATABLE].toString());
        QListIterator<QVariant> n(eData);

        while (n.hasNext()) {
            QVariantMap eDataData = n.next().value<QVariantMap>();

            out << eDataData[TIMESTAMP].toString() << separator <<
                   eDataData[VALUE].toString().replace('.', loc.decimalPoint()) << separator << "\"" <<
                   eDataData[ANNOTATION].toString() << "\"\n";
        }
    }

    out.flush();
    file.close();

    return filename;
}

Logger::~Logger()
{
    closeDatabase();
}

/* QAbstractItemModel */

Qt::ItemFlags Logger::flags(const QModelIndex& idx) const
{
    return QAbstractListModel::flags(idx) | Qt::ItemIsEditable;
}

QHash<int,QByteArray> Logger::roleNames() const
{
    QHash<int,QByteArray> roles;

    roles.insert(NameRole, NAME_ROLE);
    roles.insert(DescriptionRole, DESCRIPTION_ROLE);
    roles.insert(PlotColorRole, PLOTCOLOR_ROLE);
    roles.insert(DataTableRole, DATATABLE_ROLE);
    roles.insert(PairedTableRole, PAIREDTABLE_ROLE);
    roles.insert(VisualizeRole, VISUALIZE_ROLE);
    return roles;
}

int Logger::rowCount(const QModelIndex& parent) const
{
    return parameters.count();
}

QVariant Logger::data(const QModelIndex& idx, int role) const
{
    const int i = idx.row();
    if (i >= 0 && i < parameters.count()) {
        const QVariantMap par(parameters.at(i).toMap());
        switch (role) {
        case NameRole: return par.value(NAME).toString();
        case DescriptionRole: return par.value(DESCRIPTION).toString();
        case PlotColorRole: return par.value(PLOTCOLOR).toString();
        case DataTableRole: return par.value(DATATABLE).toString();
        case PairedTableRole: return par.value(PAIREDTABLE).toString();
        case VisualizeRole: return par.value(VISUALIZE).toBool();
        }
    }
    return QVariant();
}

bool Logger::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    const int row = idx.row();
    if (row >= 0 && row < parameters.count()) {
        DBG(row << role << value);
        QVariantMap par(parameters.at(row).toMap());
        QString sval;
        bool bval;

        switch (role) {
        case VisualizeRole:
            bval = value.toBool();
            DBG(par.value(VISUALIZE).toBool() << bval);
            if (par.value(VISUALIZE).toBool() != bval) {
                par.insert(VISUALIZE, bval);
                QVector<int> roles;
                roles.append(role);
                if (insertOrReplace(par.value(DATATABLE).toString(),
                    par.value(NAME).toString(), par.value(DESCRIPTION).toString(),
                    bval, par.value(PLOTCOLOR).toString(),
                    par.value(PAIREDTABLE).toString())) {
                    parameters.replace(row, par);

                    emit dataChanged(idx, idx, roles);
                    updateVisualizeCount();
                    updateDefaultParameter();
                }
            }
            return true;
        case PairedTableRole:
            sval = value.toString();
            DBG(par.value(PAIREDTABLE).toString() << sval);
            if (par.value(PAIREDTABLE).toString() != sval) {
                DBG("Paired table at" << row << "=>" << sval);
                QVector<int> roles;
                roles.append(role);
                if (setPairedTable(par.value(DATATABLE).toString(), sval)) {
                    par.insert(PAIREDTABLE, sval);
                    parameters.replace(row, par);
                    emit dataChanged(idx, idx, roles);
                }
            }
            return true;
        }
    }
    return false;
}
