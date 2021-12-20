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

#include "timegridmodel.h"
#include "debuglog.h"

#include <qmath.h>

#define COUNT(a) (sizeof(a)/sizeof((a)[0]))

class TimeGridModel::Grid {
public:
    enum Role {
        TimeRole = Qt::UserRole,
        TextRole,
        CoordinateRole
    };

    static const qint64 SpanSecondMSecs = Q_INT64_C(1000);
    static const qint64 SpanMinuteMSecs = Q_INT64_C(60000);
    static const qint64 SpanHourMSecs = Q_INT64_C(3600000);
    static const qint64 SpanDayMSecs = Q_INT64_C(86400000);
    static const qint64 MinSpanMonthMSecs = Q_INT64_C(2419200000);
    static const qint64 MinSpanYearMSecs = Q_INT64_C(31536000000);

    static const QString MSecsFormat;
    static const QString SecsFormat;
    static const QString TimeFormat;
    static const QString DayFormat;
    static const QString MonthFormat;
    static const QString YearMonthFormat;
    static const QString DateFormat;
    static const QString YearFormat;
    static const QString FixedFormat;

    Grid() : coord(0.0) {}
    Grid(const Grid& g) : time(g.time), coord(g.coord), text(g.text) {}
    Grid(const QDateTime& t, qreal c, QString text) : time(t), coord(c), text(text) {}

    Grid& operator=(const Grid& g) { time = g.time; coord = g.coord; text = g.text; return *this; }
    bool operator==(const Grid& g) const { return coord == g.coord && text == g.text && time == g.time; }

    static const QString& dateTimeFormat(qint64 spanMSecs);

public:
    QDateTime time;
    qreal coord;
    QString text;
};

const QString TimeGridModel::Grid::MSecsFormat("ss.zzz");
const QString TimeGridModel::Grid::SecsFormat("hh:mm::ss");
const QString TimeGridModel::Grid::TimeFormat("hh:mm");
const QString TimeGridModel::Grid::DayFormat("d MMM");
const QString TimeGridModel::Grid::MonthFormat("MMM");
const QString TimeGridModel::Grid::YearMonthFormat("yyyy MMM");
const QString TimeGridModel::Grid::DateFormat("dd.MM.yyyy");
const QString TimeGridModel::Grid::YearFormat("yyyy");
const QString TimeGridModel::Grid::FixedFormat("dd.MM.yyyy hh:mm");

const QString& TimeGridModel::Grid::dateTimeFormat(qint64 spanMSecs)
{
    return (spanMSecs <= SpanSecondMSecs) ? MSecsFormat :
        (spanMSecs <= SpanMinuteMSecs) ? SecsFormat :
        (spanMSecs <= SpanDayMSecs) ? TimeFormat :
        (spanMSecs <= MinSpanMonthMSecs) ? DayFormat :
        (spanMSecs <= MinSpanYearMSecs) ? DateFormat :
        YearFormat;
}

QDebug operator<<(QDebug out, const TimeGridModel::Grid& grid)
{
    QDebugStateSaver saver(out);
    out.nospace() << "Grid(" <<
        qPrintable(grid.time.toString(TimeGridModel::Grid::FixedFormat)) << ", " <<
        grid.coord << ", " << grid.text << ")";
    return out;
}

struct TimeGridModel::Step {
public:
    static QDateTime roundToSpan(const QDateTime& t, const QDateTime& o, qint64 spanMSecs);
    static QDateTime roundToMSecs(const QDateTime& t, const QDateTime& o, int msecs) { return roundToSpan(t, o, msecs); }
    static QDateTime roundToSecs(const QDateTime& t, const QDateTime& o, int secs) { return roundToSpan(t, o, Grid::SpanSecondMSecs * secs); }
    static QDateTime roundToMins(const QDateTime& t, const QDateTime& o, int mins) { return roundToSpan(t, o, Grid::SpanMinuteMSecs * mins); }
    static QDateTime roundToHours(const QDateTime& t, const QDateTime& o, int hours) { return roundToSpan(t, o, Grid::SpanHourMSecs * hours); }
    static QDateTime roundToDays(const QDateTime& t, const QDateTime& o, int days) { return roundToSpan(t, o, Grid::SpanDayMSecs * days); }
    static QDateTime roundToMonths(const QDateTime& t, const QDateTime&, int months);
    static QDateTime roundToYears(const QDateTime& t, const QDateTime&, int years);
    static QDateTime addMSecs(const QDateTime& t, int msecs) { return t.addMSecs(msecs); }
    static QDateTime addSecs(const QDateTime& t, int secs) { return t.addMSecs(Grid::SpanSecondMSecs * secs); }
    static QDateTime addMins(const QDateTime& t, int mins) { return t.addMSecs(Grid::SpanMinuteMSecs * mins); }
    static QDateTime addHours(const QDateTime& t, int hours) { return t.addMSecs(Grid::SpanHourMSecs * hours); }
    static QDateTime addDays(const QDateTime& t, int days) { return t.addMSecs(Grid::SpanDayMSecs * days); }
    static QDateTime addMonths(const QDateTime& t, int months) { return t.addMonths(months); }
    static QDateTime addYears(const QDateTime& t, int years) { return t.addMonths(12*years); }
    static QString formatMSecs(const QDateTime& t, const Grid* prev) { return t.toString(Grid::MSecsFormat); }
    static QString formatSecs(const QDateTime& t, const Grid* prev) { return t.toString(Grid::SecsFormat); }
    static QString formatMins(const QDateTime& t, const Grid* prev) { return t.toString(Grid::TimeFormat); }
    static QString formatHours(const QDateTime& t, const Grid* prev) { return t.toString(Grid::TimeFormat); }
    static QString formatDays(const QDateTime& t, const Grid* prev) { return t.toString(Grid::DayFormat); }
    static QString formatMonths(const QDateTime& t, const Grid* prev);
    static QString formatYears(const QDateTime& t, const Grid* prev) { return t.toString(Grid::YearFormat); }

    static const int MSecMultipliers[];
    static const int SecMultipliers[];
    static const int MinMultipliers[];
    static const int HourMultipliers[];
    static const int DayMultipliers[];
    static const int MonthMultipliers[];
    static const int YearMultipliers[];

    static const Step MSecs;
    static const Step Secs;
    static const Step Mins;
    static const Step Hours;
    static const Step Days;
    static const Step Months;
    static const Step Years;

    static const Step* AllSteps[];

public:
    QDateTime (*roundUp)(const QDateTime& t, const QDateTime& origin, int n);
    QDateTime (*add)(const QDateTime& t, int n);
    QString (*format)(const QDateTime& t, const Grid* prev);
    const qint64 minSpanMSecs;
    const char* units;
    const int* multipliers;
};

const TimeGridModel::Step TimeGridModel::Step::MSecs =
    { TimeGridModel::Step::roundToMSecs, TimeGridModel::Step::addMSecs,
      TimeGridModel::Step::formatMSecs, 1, "ms",
      TimeGridModel::Step::MSecMultipliers};
const int TimeGridModel::Step::MSecMultipliers[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 0 };

const TimeGridModel::Step TimeGridModel::Step::Secs =
    { TimeGridModel::Step::roundToSecs, TimeGridModel::Step::addSecs,
      TimeGridModel::Step::formatSecs, TimeGridModel::Grid::SpanSecondMSecs, "sec",
      TimeGridModel::Step::SecMultipliers };
const int TimeGridModel::Step::SecMultipliers[] = { 1, 2, 5, 10, 20, 30, 0 };

const TimeGridModel::Step TimeGridModel::Step::Mins =
    { TimeGridModel::Step::roundToMins, TimeGridModel::Step::addMins,
      TimeGridModel::Step::formatMins, TimeGridModel::Grid::SpanMinuteMSecs, "min",
      TimeGridModel::Step::MinMultipliers };
const int TimeGridModel::Step::MinMultipliers[] = { 1, 2, 5, 10, 20, 30, 0 };

const TimeGridModel::Step TimeGridModel::Step::Hours =
    { TimeGridModel::Step::roundToHours, TimeGridModel::Step::addHours,
      TimeGridModel::Step::formatHours, TimeGridModel::Grid::SpanHourMSecs, "hour(s)",
      TimeGridModel::Step::HourMultipliers };
const int TimeGridModel::Step::HourMultipliers[] = { 1, 2, 3, 4, 6, 12, 0 };

const TimeGridModel::Step TimeGridModel::Step::Days =
    { TimeGridModel::Step::roundToDays, TimeGridModel::Step::addDays,
      TimeGridModel::Step::formatDays, TimeGridModel::Grid::SpanDayMSecs, "day(s)",
      TimeGridModel::Step::DayMultipliers };
const int TimeGridModel::Step::DayMultipliers[] = { 1, 2, 5, 10, 0 };

const TimeGridModel::Step TimeGridModel::Step::Months =
    { TimeGridModel::Step::roundToMonths, TimeGridModel::Step::addMonths,
      TimeGridModel::Step::formatMonths, TimeGridModel::Grid::MinSpanMonthMSecs, "month(s)",
      TimeGridModel::Step::MonthMultipliers };
const int TimeGridModel::Step::MonthMultipliers[] = { 1, 3, 6, 0 };

const TimeGridModel::Step TimeGridModel::Step::Years
    { TimeGridModel::Step::roundToYears, TimeGridModel::Step::addYears,
      TimeGridModel::Step::formatYears, TimeGridModel::Grid::MinSpanYearMSecs, "year(s)",
      TimeGridModel::Step::YearMultipliers };
const int TimeGridModel::Step::YearMultipliers[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 0 };

const TimeGridModel::Step* TimeGridModel::Step::AllSteps[] = {
    &TimeGridModel::Step::MSecs,
    &TimeGridModel::Step::Secs,
    &TimeGridModel::Step::Mins,
    &TimeGridModel::Step::Hours,
    &TimeGridModel::Step::Days,
    &TimeGridModel::Step::Months,
    &TimeGridModel::Step::Years
};

QDateTime TimeGridModel::Step::roundToSpan(const QDateTime& t,
    const QDateTime& origin, qint64 spanMSecs)
{
    const qint64 msec = origin.msecsTo(t);
    const qint64 remainder = (msec % spanMSecs);
    return remainder ? t.addMSecs(spanMSecs - remainder) : t;
}

QDateTime TimeGridModel::Step::roundToMonths(const QDateTime& t,
    const QDateTime&, int months)
{
    const int msec = t.time().msecsSinceStartOfDay();
    const QDate date(t.date());
    const int m = date.month();
    const int remainder = ((m - 1) % months);
    return (msec > 0 || date.day() > 1 || remainder) ?
        QDateTime(QDate(date.year(), m, 1).addMonths(months - remainder)) : t;
}

QDateTime TimeGridModel::Step::roundToYears(const QDateTime& t,
    const QDateTime&, int years)
{
    const int msec = t.time().msecsSinceStartOfDay();
    const QDate date(t.date());
    const int y = date.year();
    const int remainder = (y % years);
    return (msec > 0 || date.day() > 1 || date.month() > 1 || remainder) ?
        QDateTime(QDate(y, 1, 1).addYears(years - remainder)) : t;
}

QString TimeGridModel::Step::formatMonths(const QDateTime& t, const Grid* prev)
{
    return (prev && prev->time.date().year() != t.date().year()) ?
        t.toString(Grid::YearMonthFormat) : t.toString(Grid::MonthFormat);
}

TimeGridModel::TimeGridModel(QObject* parent) :
    QAbstractListModel(parent),
    m_size(1.0),
    m_minCount(1),
    m_maxCount(10),
    m_timeOrigin(QDateTime::fromMSecsSinceEpoch(0)),
    m_fixedGrids(false)
{
}

TimeGridModel::~TimeGridModel()
{
}

void TimeGridModel::setSize(qreal size)
{
    if (m_size != size) {
        m_size = size;
        DBG(size);
        updateGrids();
        emit sizeChanged();
    }
}

void TimeGridModel::setMinCount(int count)
{
    if (m_minCount != count) {
        m_minCount = count;
        DBG(count);
        updateGrids();
        minCountChanged();
    }
}

void TimeGridModel::setMaxCount(int count)
{
    if (m_maxCount != count) {
        m_maxCount = count;
        DBG(count);
        updateGrids();
        maxCountChanged();
    }
}

void TimeGridModel::setTimeStart(QDateTime t)
{
    if (m_timeStart != t) {
        m_timeStart = t;
        DBG(t);
        updateGrids();
        emit timeStartChanged();
    }
}

void TimeGridModel::setTimeEnd(QDateTime t)
{
    if (m_timeEnd != t) {
        m_timeEnd = t;
        DBG(t);
        updateGrids();
        emit timeEndChanged();
    }
}

void TimeGridModel::setTimeOrigin(QDateTime t)
{
    if (m_timeOrigin != t) {
        m_timeOrigin = t;
        DBG(t);
        updateGrids();
        emit timeOriginChanged();
    }
}

void TimeGridModel::setFixedGrids(bool fixed)
{
    if (m_fixedGrids != fixed) {
        m_fixedGrids = fixed;
        DBG(fixed);
        updateGrids();
        fixedGridsChanged();
    }
}

bool TimeGridModel::makeGrids(QVector<Grid>* grids, const Step* step, int n) const
{
    grids->resize(0);
    const qint64 spanMSecs = m_timeStart.msecsTo(m_timeEnd);
    QDateTime t(step->roundUp(m_timeStart, m_timeOrigin, n));
    for (int i = 0; t < m_timeEnd; i++, t = step->add(t, n)) {
        if (grids->count() >= m_maxCount) {
            DBG("step" << n << step->units << "is too small");
            return false;
        }
        const qreal coord = qRound(m_size * m_timeStart.msecsTo(t) / spanMSecs);
        const QString text(step->format(t, grids->isEmpty() ? Q_NULLPTR : &grids->last()));
        const Grid grid(t, coord, text);
        grids->append(grid);
        DBG(grids->count() << ":" << grid);
    }
    if (grids->count() >= m_minCount) {
        DBG("step" << n << step->units << "is ok");
        return true;
    } else {
        DBG("step" << n << step->units << "is too large");
        return false;
    }
}

bool TimeGridModel::makeGrids(QVector<Grid>* grids, const Step* step) const
{
    const qint64 spanMSecs = m_timeStart.msecsTo(m_timeEnd);
    for (int i = 0; step->multipliers[i]; i++) {
        const int m = step->multipliers[i];
        if (m * step->minSpanMSecs * m_size > spanMSecs) {
            if (makeGrids(grids, step, step->multipliers[i])) {
                return true;
            }
            grids->resize(0);
        } else {
            DBG("step" << m  << step->units << "is too small");
        }
    }
    return false;
}

void TimeGridModel::updateGrids()
{
    QVector<Grid> grids;
    if (m_size > 0 && m_maxCount > 0 && m_maxCount >= m_minCount &&
        m_timeEnd.isValid() && m_timeStart.isValid() &&
        m_timeEnd > m_timeStart) {
        const qint64 spanMSecs = m_timeStart.msecsTo(m_timeEnd);
        if (m_fixedGrids) {
            DBG("grid" << m_timeStart << ".." << m_timeEnd);
            int i, k = COUNT(Step::AllSteps) - 1;
            const Step* step = Step::AllSteps[k];
            while (spanMSecs < step->minSpanMSecs && k > 0) {
                DBG("skipping" << step->units);
                step = Step::AllSteps[--k];
            }
            // We only need to compare labels if we have smaller steps
            // (which is usually the case)
            QString lastLabel;
            bool repeats = false;
            const Step* smallerStep = (k > 0) ? Step::AllSteps[k - 1] : Q_NULLPTR;
            for (i = 0; i < m_maxCount; i++) {
                const qint64 offsetMSecs = (i + 1) * spanMSecs / (m_maxCount + 1);
                const qreal coord = m_size * offsetMSecs / spanMSecs;
                const QDateTime t(m_timeStart.addMSecs(offsetMSecs));
                const QString text(step->format(t, grids.isEmpty() ? Q_NULLPTR : &grids.last()));
                const Grid grid(t, coord, text);
                grids.append(grid);
                if (smallerStep && !repeats) {
                    if (lastLabel == text) {
                        DBG(grids.count() << ":" << grid << "<== repeated label");
                        repeats = true;
                        continue;
                    } else {
                        lastLabel = text;
                    }
                }
                DBG(grids.count() << ":" << grid);
            }
            if (repeats) {
                Grid* gridData = grids.data();
                for (i = 0; i < m_maxCount; i++) {
                    gridData[i].text = smallerStep->format(gridData[i].time,
                        (i > 0) ? (gridData + (i - 1)) : Q_NULLPTR);
                    DBG((i + 1) << ":" << gridData[i]);
                }
            }
        } else {
            // Dynamic grids
            int i = COUNT(Step::AllSteps) - 1;
            while (i >= 0) {
                const Step* step = Step::AllSteps[i--];
                if (spanMSecs > step->minSpanMSecs) {
                    if (makeGrids(&grids, step)) {
                        if (i >= 0 && grids.count() < m_maxCount) {
                            // Check if a smaller grid fits even better
                            QVector<Grid> smaller;
                            DBG("trying" << Step::AllSteps[i]->units);
                            if (makeGrids(&smaller, Step::AllSteps[i])) {
                                grids = smaller;
                            }
                        }
                        break;
                    }
                    grids.resize(0);
                } else {
                    DBG("skipping" << step->units);
                }
            }
        }
    }

    if (m_grids != grids) {
        beginResetModel();
        m_grids = grids;
        endResetModel();
    }
}

/* QAbstractItemModel */

QHash<int,QByteArray> TimeGridModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles.insert(Grid::TimeRole, "time");
    roles.insert(Grid::TextRole, "text");
    roles.insert(Grid::CoordinateRole, "coordinate");
    return roles;
}

int TimeGridModel::rowCount(const QModelIndex& parent) const
{
    return m_grids.count();
}

QVariant TimeGridModel::data(const QModelIndex& idx, int role) const
{
    const int row = idx.row();
    if (row >= 0 && row < m_grids.count()) {
        const Grid& grid = m_grids.at(row);
        switch ((Grid::Role)role) {
        case Grid::TimeRole: return grid.time;
        case Grid::TextRole: return grid.text;
        case Grid::CoordinateRole: return grid.coord;
        }
    }
    return QVariant();
}
