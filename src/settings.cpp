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

#include "settings.h"

#define CONFIG_ROOT  "/apps/harbour-valuelogger2/"
#define DCONF_KEY(x)  CONFIG_ROOT x

const QString Settings::ConfigRoot(CONFIG_ROOT);

Settings::Settings(QObject* parent) :
    QObject(parent),
    m_verticalGridLinesStyle(new MGConfItem(DCONF_KEY("verticalGridLinesStyle"), this)),
    m_horizontalGridLinesStyle(new MGConfItem(DCONF_KEY("horizontalGridLinesStyle"), this)),
    m_topGridLabels(new MGConfItem(DCONF_KEY("topGridLabels"), this)),
    m_leftGridLabels(new MGConfItem(DCONF_KEY("leftGridLabels"), this)),
    m_rightGridLabels(new MGConfItem(DCONF_KEY("rightGridLabels"), this)),
    m_showGraphOnCover(new MGConfItem(DCONF_KEY("showGraphOnCover"), this))
{
    connect(m_verticalGridLinesStyle, SIGNAL(valueChanged()), SIGNAL(verticalGridLinesStyleChanged()));
    connect(m_horizontalGridLinesStyle, SIGNAL(valueChanged()), SIGNAL(horizontalGridLinesStyleChanged()));
    connect(m_topGridLabels, SIGNAL(valueChanged()), SIGNAL(topGridLabelsChanged()));
    connect(m_leftGridLabels, SIGNAL(valueChanged()), SIGNAL(leftGridLabelsChanged()));
    connect(m_rightGridLabels, SIGNAL(valueChanged()), SIGNAL(rightGridLabelsChanged()));
    connect(m_showGraphOnCover, SIGNAL(valueChanged()), SIGNAL(showGraphOnCoverChanged()));
}

/* Callback for qmlRegisterSingletonType<Settings> */
QObject* Settings::createSingleton(QQmlEngine* engine, QJSEngine* js)
{
    return new Settings;
}
