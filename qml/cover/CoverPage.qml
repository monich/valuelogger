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

    Item {
        id: iconItem

        readonly property int padding: Theme.paddingSmall

        y: x
        width: parent.width / 2 + 2 * padding
        height: width
        anchors.horizontalCenter: parent.horizontalCenter
        visible: !showGraph

        HighlightIcon {
            source: showGraph ? "" : "../images/icon-background.svg"
            highlightColor: Theme.primaryColor
            anchors.centerIn: parent
            sourceSize.width: icon.width + parent.padding
            sourceSize.height: icon.height + parent.padding
            opacity: 0.2
        }

        Image {
            id: icon

            source: showGraph ? "" : Qt.resolvedUrl("../images/harbour-valuelogger2.svg")
            anchors.centerIn: parent
            sourceSize.width: parent.width - 2 * parent.padding
        }
    }

    Loader {
        id: graphItem

        y: x
        width: parent.width - 2 * Theme.paddingLarge
        height: width
        anchors.horizontalCenter: parent.horizontalCenter
        active: Settings.showGraphOnCover
        sourceComponent: Component {
            Item {
                anchors.fill: parent
                visible: showGraph

                Connections {
                    target: Logger
                    onTableUpdated: {
                        if (table == defaultParameterData.dataTable) {
                            defaultParameterData.reset()
                        }
                    }
                }

                DataModel {
                    id: defaultParameterData

                    readonly property real valueSpan: maxValue - minValue

                    dataTable: Logger.defaultParameterTable

                    onModelReset: graph.update()
                }

                Graph {
                    id: graph

                    readonly property real extraValueRoom: height ? (lineWidth / height * defaultParameterData.valueSpan) : 0

                    anchors.centerIn: parent
                    width: parent.width - 2 * graphBorder.border.width
                    height: parent.height - 2 * graphBorder.border.width
                    model: defaultParameterData
                    minValue: defaultParameterData.minValue - extraValueRoom
                    maxValue: defaultParameterData.maxValue + extraValueRoom
                    minTime: defaultParameterData.minTime
                    maxTime: defaultParameterData.maxTime
                    lineWidth: thickLine
                    nodeMarks: false
                    color: Logger.defaultParameterColor
                }

                Repeater {
                    id: verticalGrid

                    readonly property int gridCount: 5
                    model: gridCount - 1
                    delegate: VDashLine {
                        x: graph.x + Math.round((index + 1) * graph.width / verticalGrid.gridCount - width / 2)
                        y: graph.y
                        width: thinLine
                        height: graph.height
                        dashSize: 2 * width
                        color: Theme.primaryColor
                        opacity: 0.4
                    }
                }

                Repeater {
                    id: horizontalGrid

                    readonly property int gridCount: 5
                    model: gridCount - 1
                    delegate: HDashLine {
                        x: graph.x
                        y: graph.x + Math.round((index + 1) * graph.height / horizontalGrid.gridCount - height / 2)
                        width: graph.width
                        height: thinLine
                        dashSize: 2 * height
                        color: Theme.primaryColor
                        opacity: 0.4
                    }
                }

                Rectangle {
                    id: graphBorder

                    anchors.fill: parent
                    opacity: 0.6
                    color: "transparent"
                    border {
                        width: thickLine
                        color: Theme.primaryColor
                    }
                }
            }
        }
    }

    Label {
        id: label

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: showGraph ? graphItem.bottom : iconItem.bottom
            bottom: parent.bottom
            bottomMargin: (Theme.itemSizeSmall + Theme.iconSizeSmall)/2/cover.parent.scale
        }
        width: Math.min(implicitWidth, parent.width - 2 * Theme.paddingLarge)
        verticalAlignment: Text.AlignVCenter
        truncationMode: TruncationMode.Fade
        minimumPixelSize: Theme.fontSizeExtraSmall
        fontSizeMode: Text.Fit
        color: Theme.highlightColor
        text: Logger.defaultParameterName ? Logger.defaultParameterName : "Value logger"
    }

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
