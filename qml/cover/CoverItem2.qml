import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

import "../components"

CoverItemBase {
    id: thisItem

    graphBottomY: graphItem.x + graphItem.height

    Loader {
        id: graphItem

        y: x
        width: parent.width - 2 * Theme.paddingLarge
        height: width
        anchors.horizontalCenter: parent.horizontalCenter
        active: showGraph
        sourceComponent: Component {
            Item {
                anchors.fill: parent

                Graph {
                    id: graph

                    readonly property real extraValueRoom: height ? (lineWidth / height * (model.maxValue - model.minValue)) : 0

                    anchors.centerIn: parent
                    width: parent.width - 2 * graphBorder.border.width
                    height: parent.height - 2 * graphBorder.border.width
                    model: graphModel
                    minValue: model.minValue - extraValueRoom
                    maxValue: model.maxValue + extraValueRoom
                    minTime: model.minTime
                    maxTime: model.maxTime
                    lineWidth: thickLine
                    nodeMarks: false
                    color: graphColor
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
}
