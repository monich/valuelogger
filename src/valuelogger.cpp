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

#include "graph.h"
#include "gridmodel.h"
#include "logger.h"
#include "datamodel.h"
#include "debuglog.h"

#include <sailfishapp.h>

#include <QScopedPointer>
#include <QQuickView>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QTranslator>
#include <QLocale>

// Configures debug log at run time
class DebugLog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled CONSTANT)
public:
    DebugLog(QObject* parent = Q_NULLPTR) : QObject(parent) {}
    static QObject* createSingleton(QQmlEngine*, QJSEngine*) { return new DebugLog(); }
    bool isEnabled() const {
        QByteArray val(qgetenv("DEBUG"));
        if (val == "0") {
            return false;
        } else if (val == "1") {
            return true;
        }
        return LOG_DBG;
    }
};

int main(int argc, char *argv[])
{
    /* Initialise random number generator */
    qsrand(QDateTime::currentDateTime().toTime_t());

    const char *uri = "harbour.valuelogger";
    qmlRegisterType<Graph>(uri, 1, 0, "Graph");
    qmlRegisterType<GridModel>(uri, 1, 0, "GridModel");
    qmlRegisterType<DataModel>(uri, 1, 0, "DataModel");
    qmlRegisterSingletonType<Logger>(uri, 1, 0, "Logger", Logger::createSingleton);
    qmlRegisterSingletonType<DebugLog>(uri, 1, 0, "DebugLog", DebugLog::createSingleton);

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));

    QLocale locale;
    QTranslator translator;
    const QString transDir = SailfishApp::pathTo("translations").toLocalFile();
    if (translator.load(locale, "harbour-valuelogger2", "_", transDir)) {
        app->installTranslator(&translator);
    } else {
        DBG("Failed to load translator for" << locale);
    }

    QScopedPointer<QQuickView> view(SailfishApp::createView());
    // Enable multisampling
    QSurfaceFormat format = view->format();
    format.setSamples(16);
    view->setFormat(format);
    view->setSource(SailfishApp::pathTo("qml/valuelogger.qml"));
    view->show();

    return app->exec();
}

#include "valuelogger.moc"
