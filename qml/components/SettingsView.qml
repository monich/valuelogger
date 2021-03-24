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

        SectionHeader { text: qsTr("Horizontal grid lines") }

        ComboBox {
            width: parent.width
            label: qsTr("Position")
            currentItem: (Settings.horizontalGridLinesStyle === Settings.GridLinesDynamic) ? horizontalGridLinesDynamic :
                (Settings.horizontalGridLinesStyle = Settings.GridLinesFixed) ? horizontalGridLinesFixed : null
            menu: ContextMenu {
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
        Grid {
            width: parent.width
            columns: landscapeLayout ? 2 : 1

            readonly property real columnWidth: width/columns

            TextSwitch {
                width: parent.columnWidth
                text: qsTr("Left labels")
                automaticCheck: false
                checked: Settings.leftGridLabels
                onClicked: Settings.leftGridLabels = !Settings.leftGridLabels
            }

            TextSwitch {
                width: parent.columnWidth
                text: qsTr("Right labels")
                automaticCheck: false
                checked: Settings.rightGridLabels
                onClicked: Settings.rightGridLabels = !Settings.rightGridLabels
            }
        }
    }
}
