import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

import "../components"

CoverBackground {
    id: cover

    signal addValue()
    signal plotSelected()

    readonly property bool darkOnLight: 'colorScheme' in Theme && Theme.colorScheme === Theme.DarkOnLight
    readonly property url plotIcon: Qt.resolvedUrl("../images/" + (darkOnLight ? "icon-cover-plot-dark.svg" : "icon-cover-plot.svg"))
    readonly property int thinLine: Math.max(1, Math.floor(Theme.paddingSmall/4))
    readonly property int thickLine: Math.max(2, Math.floor(Theme.paddingSmall/2))
    readonly property bool showGraph: Settings.showGraphOnCover && Logger.defaultParameterTable !== ""
    readonly property string title: Logger.defaultParameterName ? Logger.defaultParameterName : "Value logger"

    property alias coverItem: coverItemLoader.item

    DataTableModel {
        id: defaultParameterData

        dataTable: Logger.defaultParameterTable
        onModelReset: repaintTimer.start()
    }

    Timer {
        id: repaintTimer

        interval: 500
        onTriggered: coverItem.repaintGraph()
    }

    Loader {
        id: coverItemLoader

        anchors.fill: cover
        source: Qt.resolvedUrl(Settings.coverItem)
    }

    Binding { target: coverItem;  property: "coverScale"; value: cover.parent.scale }
    Binding { target: coverItem;  property: "showGraph"; value: showGraph }
    Binding { target: coverItem;  property: "graphColor"; value: Logger.defaultParameterColor }
    Binding { target: coverItem;  property: "graphModel"; value: defaultParameterData }
    Binding { target: coverItem;  property: "title"; value: title }

    CoverActionList {
        enabled: Logger.visualizeCount === 1

        CoverAction {
            iconSource: "image://theme/icon-cover-new"
            onTriggered: addValue()
        }
        CoverAction {
            iconSource: plotIcon
            onTriggered: cover.plotSelected()
        }
    }

    CoverActionList {
        enabled: Logger.visualizeCount > 1

        CoverAction {
            iconSource: plotIcon
            onTriggered: cover.plotSelected()
        }
    }
}
