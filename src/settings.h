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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariant>
#include <MGConfItem>

class QQmlEngine;
class QJSEngine;

class Settings : public QObject
{
    Q_OBJECT
    Q_ENUMS(GridLinesStyle)
    Q_PROPERTY(QString configRoot READ getConfigRoot CONSTANT)
    Q_PROPERTY(GridLinesStyle verticalGridLinesStyle READ getVerticalGridLinesStyle WRITE setVerticalGridLinesStyle NOTIFY verticalGridLinesStyleChanged)
    Q_PROPERTY(GridLinesStyle horizontalGridLinesStyle READ getHorizontalGridLinesStyle WRITE setHorizontalGridLinesStyle NOTIFY horizontalGridLinesStyleChanged)
    Q_PROPERTY(bool topGridLabels READ getTopGridLabels WRITE setTopGridLabels NOTIFY topGridLabelsChanged)
    Q_PROPERTY(bool leftGridLabels READ getLeftGridLabels WRITE setLeftGridLabels NOTIFY leftGridLabelsChanged)
    Q_PROPERTY(bool rightGridLabels READ getRightGridLabels WRITE setRightGridLabels NOTIFY rightGridLabelsChanged)
    Q_PROPERTY(bool showGraphOnCover READ getShowGraphOnCover WRITE setShowGraphOnCover NOTIFY showGraphOnCoverChanged)
    Q_PROPERTY(int coverStyle READ getCoverStyle WRITE setCoverStyle NOTIFY coverStyleChanged)
    Q_PROPERTY(QString coverItem READ getCoverItem NOTIFY coverStyleChanged)
    Q_PROPERTY(QStringList coverItems READ getCoverItems CONSTANT)
    Q_PROPERTY(QVariantList sampleData READ getSampleData CONSTANT)

    static const QString ConfigRoot;
    static const QStringList CoverItems;
    static const QVariantList SampleData;

public:
    enum GridLinesStyle {
        GridLinesDynamic,
        GridLinesFixed
    };

    explicit Settings(QObject* parent = Q_NULLPTR);

    /* Callback for qmlRegisterSingletonType<Settings> */
    static QObject* createSingleton(QQmlEngine* engine, QJSEngine* js);

    /* Static helpers */
    static const QString& getConfigRoot() { return ConfigRoot; }
    static const QStringList& getCoverItems() { return CoverItems; }
    static const QVariantList& getSampleData() { return SampleData; }
    static int validateCoverStyle(int style);

    /* Getters and setters */
    GridLinesStyle getVerticalGridLinesStyle() const { return (GridLinesStyle) m_verticalGridLinesStyle->value((int) GridLinesDynamic).toInt(); }
    void setVerticalGridLinesStyle(GridLinesStyle value) { m_verticalGridLinesStyle->set((int) value); }

    GridLinesStyle getHorizontalGridLinesStyle() const { return (GridLinesStyle) m_horizontalGridLinesStyle->value((int) GridLinesDynamic).toInt(); }
    void setHorizontalGridLinesStyle(GridLinesStyle value) { m_horizontalGridLinesStyle->set((int) value); }

    bool getTopGridLabels() const { return m_topGridLabels->value(true).toBool(); }
    void setTopGridLabels(bool value) { m_topGridLabels->set(value); }

    bool getLeftGridLabels() const { return m_leftGridLabels->value(true).toBool(); }
    void setLeftGridLabels(bool value) { m_leftGridLabels->set(value); }

    bool getRightGridLabels() const { return m_rightGridLabels->value(true).toBool(); }
    void setRightGridLabels(bool value) { m_rightGridLabels->set(value); }

    bool getShowGraphOnCover() const { return m_showGraphOnCover->value(true).toBool(); }
    void setShowGraphOnCover(bool value) { m_showGraphOnCover->set(value); }

    int getCoverStyle() const;
    void setCoverStyle(int style);
    const QString& getCoverItem() const { return CoverItems.at(getCoverStyle()); }

private:
    static QVariantList createSampleData();

signals:
    void verticalGridLinesStyleChanged();
    void horizontalGridLinesStyleChanged();
    void topGridLabelsChanged();
    void leftGridLabelsChanged();
    void rightGridLabelsChanged();
    void showGraphOnCoverChanged();
    void coverStyleChanged();

private:
    MGConfItem* m_verticalGridLinesStyle;
    MGConfItem* m_horizontalGridLinesStyle;
    MGConfItem* m_topGridLabels;
    MGConfItem* m_leftGridLabels;
    MGConfItem* m_rightGridLabels;
    MGConfItem* m_showGraphOnCover;
    MGConfItem* m_coverStyle;
};

#endif // SETTINGS_H
