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

#include <QColor>
#include <QFile>
#include <QLocale>
#include <QStandardPaths>

enum LoggerRole {
    NameRole = Qt::UserRole,
    DescriptionRole,
    PlotColorRole,
    DataTableRole,
    PairedTableRole,
    VisualizeRole
};

namespace {
    const QString PARAMETER(PARAMETER_COL);
    const QString NAME(NAME_ROLE);
    const QString DESCRIPTION(DESCRIPTION_ROLE);
    const QString VISUALIZE(VISUALIZE_ROLE);
    const QString PLOTCOLOR(PLOTCOLOR_ROLE);
    const QString DATATABLE(DATATABLE_ROLE);
    const QString PAIREDTABLE(PAIREDTABLE_ROLE);

    const QString KEY(DATA_KEY_ROLE);
    const QString TIMESTAMP(DATA_TIMESTAMP_ROLE);
    const QString VALUE(DATA_VALUE_ROLE);
    const QString ANNOTATION(DATA_ANNOTATION_ROLE);
}

Logger::Logger(QObject *parent) :
    QAbstractListModel(parent)
{
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
 * Add new data entry to a parameter
 */

QString Logger::addData(QString table, QString value, QString annotation, QString timestamp)
{
    return db.addData(table, QString(), value, annotation, timestamp);
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
 * Read all parameters (to be shown on mainpage listview
 */

QVariantList Logger::readParameters()
{
    return db.readParameters();
}

QString Logger::addParameter(QString name, QString description, bool visualize, QColor plotcolor)
{
    DBG("Adding entry:" << name << "-" << description << "color" << plotcolor);

    const QString colorName(plotcolor.name());
    const QString datatable(db.addParameter(name, description, visualize, colorName));
    if (!datatable.isEmpty()) {
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
    }
    return datatable;
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
            if (db.insertOrReplace(par.value(DATATABLE).toString(), name, description, visualize, colorName, pairedtable)) {
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

void Logger::deleteParameterAt(int row)
{
    if (row >= 0 && row < parameters.count()) {
        const QVariantMap deleted(parameters.at(row).toMap());
        const QString dataTable(deleted.value(DATATABLE).toString());

        DBG("Deleting entry at" << row << ":" << deleted.value(NAME).toString());
        db.deleteParameter(dataTable);

        beginRemoveRows(QModelIndex(), row, row);
        parameters.removeAt(row);
        endRemoveRows();

        for (int i = parameters.count() - 1; i >= 0; i--) {
            const QVariantMap par(parameters.at(i).toMap());
            if (par.value(PAIREDTABLE).toString() == dataTable) {
                if (db.setPairedTable(par.value(DATATABLE).toString(), QString())) {
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

    const QVariantList parList = readParameters();
    QListIterator<QVariant> i(parList);

    while (i.hasNext()) {
        const QVariantMap par(i.next().value<QVariantMap>());

        out << par.value(NAME).toString() << separator << par.value(DESCRIPTION).toString() << "\n";

        const QVariantList dataList(db.readData(par.value(DATATABLE).toString()));
        QListIterator<QVariant> n(dataList);

        while (n.hasNext()) {
            const QVariantMap data(n.next().value<QVariantMap>());
            out << data.value(TIMESTAMP).toString() << separator <<
                   data.value(VALUE).toString().replace('.', loc.decimalPoint()) << separator << "\"" <<
                   data.value(ANNOTATION).toString() << "\"\n";
        }
    }

    out.flush();
    file.close();

    return filename;
}

Logger::~Logger()
{
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
                if (db.insertOrReplace(par.value(DATATABLE).toString(),
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
                if (db.setPairedTable(par.value(DATATABLE).toString(), sval)) {
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
