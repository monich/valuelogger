import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

SilicaFlickable  {
    anchors.fill: parent
    contentHeight: column.height + Theme.paddingLarge

    readonly property bool landscapeLayout: (width > height && Screen.sizeCategory > Screen.Small) || Screen.sizeCategory > Screen.Medium

    Column {
        id: column

        width: parent.width

        PageHeader { title: qsTr("Settings") }

        SectionHeader { text: qsTr("Graph") }

        TextSwitch {
            text: qsTr("Smooth")
            automaticCheck: false
            checked: Settings.smoothGraph
            onClicked: Settings.smoothGraph = !Settings.smoothGraph
        }

        SectionHeader { text: qsTr("Grid") }

        Grid {
            width: parent.width
            columns: landscapeLayout ? 2 : 1

            readonly property real columnWidth: width/columns

            ComboBox {
                id: timeAxisComboBox

                width: parent.columnWidth
                label: qsTr("Time axis")
                currentItem: (Settings.verticalGridLinesStyle === Settings.GridLinesDynamic) ? verticalGridLinesDynamic :
                    (Settings.verticalGridLinesStyle = Settings.GridLinesFixed) ? verticalGridLinesFixed : null
                menu: ContextMenu {
                    x: 0
                    width: timeAxisComboBox.width

                    MenuItem {
                        id: verticalGridLinesDynamic

                        text: qsTr("Dynamic")
                        onClicked: Settings.verticalGridLinesStyle = Settings.GridLinesDynamic
                    }
                    MenuItem {
                        id: verticalGridLinesFixed

                        text: qsTr("Fixed")
                        onClicked: Settings.verticalGridLinesStyle = Settings.GridLinesFixed
                    }
                }
            }

            TextSwitch {
                width: parent.columnWidth
                text: qsTr("Time labels")
                automaticCheck: false
                checked: Settings.topGridLabels
                onClicked: Settings.topGridLabels = !Settings.topGridLabels
            }
        }

        Grid {
            width: parent.width
            columns: landscapeLayout ? 2 : 1

            readonly property real columnWidth: width/columns

            ComboBox {
                id: valueAxisComboBox

                width: parent.columnWidth
                label: qsTr("Value axis")
                currentItem: (Settings.horizontalGridLinesStyle === Settings.GridLinesDynamic) ? horizontalGridLinesDynamic :
                    (Settings.horizontalGridLinesStyle = Settings.GridLinesFixed) ? horizontalGridLinesFixed : null
                menu: ContextMenu {
                    x: 0
                    width: valueAxisComboBox.width

                    MenuItem {
                        id: horizontalGridLinesDynamic

                        text: qsTr("Dynamic")
                        onClicked: Settings.horizontalGridLinesStyle = Settings.GridLinesDynamic
                    }
                    MenuItem {
                        id: horizontalGridLinesFixed

                        text: qsTr("Fixed")
                        onClicked: Settings.horizontalGridLinesStyle = Settings.GridLinesFixed
                    }
                }
            }

            Column {
                width: parent.columnWidth

                TextSwitch {
                    width: parent.width
                    text: qsTr("Left labels")
                    automaticCheck: false
                    checked: Settings.leftGridLabels
                    onClicked: Settings.leftGridLabels = !Settings.leftGridLabels
                }

                TextSwitch {
                    width: parent.width
                    text: qsTr("Right labels")
                    automaticCheck: false
                    checked: Settings.rightGridLabels
                    onClicked: Settings.rightGridLabels = !Settings.rightGridLabels
                }
            }
        }

        SectionHeader { text: qsTr("Cover") }

        DataModel {
            id: sampleModel

            rawData: Settings.sampleData
        }

        Grid {
            x: Theme.horizontalPageMargin
            columnSpacing: Theme.paddingLarge
            rowSpacing: Theme.paddingLarge
            columns: Math.floor((parent.width - 2 * x + columnSpacing)/(Theme.coverSizeSmall.width + columnSpacing))

            Repeater {
                model: Settings.coverItems
                delegate: CoverPreview {
                    dataModel: sampleModel
                    source: "../cover/" + modelData
                    selected: (Settings.coverStyle === model.index)
                    onClicked: Settings.coverStyle = model.index
                }
            }
        }
    }

    VerticalScrollDecorator { }
}
