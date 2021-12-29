import QtQuick 2.0
import QtGraphicalEffects 1.0
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

                Rectangle {
                    id: background

                    x: thickLine
                    y: thickLine
                    width: parent.width - 2 * x
                    height: parent.height - 2 * y
                    color: darkOnLight ? "white" : "black"
                    radius: Theme.paddingSmall - thickLine
                    opacity: 0.2
                }

                Rectangle {
                    id: mask

                    anchors.fill: background
                    color: background.color
                    radius: background.radius
                    visible: false
                }

                Connections {
                    target: thisItem
                    onUpdateGraph: graph.update()
                }

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
                    visible: false
                }

                OpacityMask {
                    anchors.fill: graph
                    source: graph
                    maskSource: mask
                }

                ShaderEffectSource {
                    id: grid

                    readonly property color color: Theme.primaryColor

                    anchors.fill: parent
                    opacity: 0.4
                    sourceItem: Item {
                        width: grid.width
                        height: grid.height

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
                                color: grid.color
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
                                color: grid.color
                            }
                        }
                    }
                }

                Rectangle {
                    id: graphBorder

                    anchors.fill: parent
                    opacity: 0.6
                    color: "transparent"
                    radius: Theme.paddingSmall
                    border {
                        width: thickLine
                        color: Theme.primaryColor
                    }
                }
            }
        }
    }
}
