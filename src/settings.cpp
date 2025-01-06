/*
Copyright (c) 2021-2025 Slava Monich <slava@monich.com>

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

#include "settings.h"
#include "datamodel.h"
#include "debuglog.h"

#define CONFIG_ROOT  "/apps/harbour-valuelogger2/"
#define DCONF_KEY(x)  CONFIG_ROOT x

namespace {
    const QString KEY(ROLE_KEY_ROLE);
    const QString TIMESTAMP(ROLE_TIMESTAMP_ROLE);
    const QString VALUE(ROLE_VALUE_ROLE);
    const int STATIC_COVER_STYLE = 0;
    const int DEFAULT_COVER_STYLE = 2;
}

const QString Settings::ConfigRoot(CONFIG_ROOT);
const QVariantList Settings::SampleData(Settings::createSampleData());
const QStringList Settings::CoverItems(QStringList(
    QStringLiteral("CoverItem1.qml")) <<
    QStringLiteral("CoverItem2.qml") <<
    QStringLiteral("CoverItem3.qml"));

Settings::Settings(QObject* parent) :
    QObject(parent),
    m_verticalGridLinesStyle(new MGConfItem(DCONF_KEY("verticalGridLinesStyle"), this)),
    m_horizontalGridLinesStyle(new MGConfItem(DCONF_KEY("horizontalGridLinesStyle"), this)),
    m_topGridLabels(new MGConfItem(DCONF_KEY("topGridLabels"), this)),
    m_leftGridLabels(new MGConfItem(DCONF_KEY("leftGridLabels"), this)),
    m_rightGridLabels(new MGConfItem(DCONF_KEY("rightGridLabels"), this)),
    m_showGraphOnCover(new MGConfItem(DCONF_KEY("showGraphOnCover"), this)),
    m_smoothGraph(new MGConfItem(DCONF_KEY("smoothGraph"), this)),
    m_coverStyle(new MGConfItem(DCONF_KEY("coverStyle"), this))
{
    connect(m_verticalGridLinesStyle, SIGNAL(valueChanged()), SIGNAL(verticalGridLinesStyleChanged()));
    connect(m_horizontalGridLinesStyle, SIGNAL(valueChanged()), SIGNAL(horizontalGridLinesStyleChanged()));
    connect(m_topGridLabels, SIGNAL(valueChanged()), SIGNAL(topGridLabelsChanged()));
    connect(m_leftGridLabels, SIGNAL(valueChanged()), SIGNAL(leftGridLabelsChanged()));
    connect(m_rightGridLabels, SIGNAL(valueChanged()), SIGNAL(rightGridLabelsChanged()));
    connect(m_showGraphOnCover, SIGNAL(valueChanged()), SIGNAL(showGraphOnCoverChanged()));
    connect(m_smoothGraph, SIGNAL(valueChanged()), SIGNAL(smoothGraphChanged()));
    connect(m_showGraphOnCover, SIGNAL(valueChanged()), SIGNAL(coverStyleChanged()));
    connect(m_smoothGraph, SIGNAL(valueChanged()), SIGNAL(coverStyleChanged()));
    connect(m_coverStyle, SIGNAL(valueChanged()), SIGNAL(coverStyleChanged()));
}

/* Callback for qmlRegisterSingletonType<Settings> */
QObject* Settings::createSingleton(QQmlEngine*, QJSEngine*)
{
    return new Settings();
}

QVariantList Settings::createSampleData()
{
    QVariantList list;
    QVariantMap entry;

    entry.insert(KEY, 0);
    entry.insert(VALUE, 275);
    entry.insert(TIMESTAMP, "2020-11-01 00:00:00");
    list.append(entry);
    entry.clear();

    entry.insert(KEY, 1);
    entry.insert(VALUE, 284);
    entry.insert(TIMESTAMP, "2020-12-01 00:00:00");
    list.append(entry);
    entry.clear();

    entry.insert(KEY, 2);
    entry.insert(VALUE, 273);
    entry.insert(TIMESTAMP, "2021-01-01 00:00:00");
    list.append(entry);
    entry.clear();

    entry.insert(KEY, 3);
    entry.insert(VALUE, 289);
    entry.insert(TIMESTAMP, "2021-02-01 00:00:00");
    list.append(entry);
    return list;
}

int Settings::validateCoverStyle(int style)
{
    return (style >= 0 && style < CoverItems.size()) ? style : DEFAULT_COVER_STYLE;
}

int Settings::getCoverStyle() const
{
    if (getShowGraphOnCover()) {
        int style = validateCoverStyle(m_coverStyle->value(DEFAULT_COVER_STYLE).toInt());
        return (style != STATIC_COVER_STYLE) ? style : DEFAULT_COVER_STYLE;
    } else {
        return STATIC_COVER_STYLE;
    }
}

void Settings::setCoverStyle(int style)
{
    const int validatedStyle = validateCoverStyle(style);
    DBG(validatedStyle);
    setShowGraphOnCover(validatedStyle != STATIC_COVER_STYLE);
    m_coverStyle->set(validatedStyle);
}
