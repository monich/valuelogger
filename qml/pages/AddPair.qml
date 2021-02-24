import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    property string pairFirstTable: ""
    property string pairSecondTable: ""

    DialogHeader {
        id: dialogHeader

        acceptText: pairSecondTable === "" ? qsTr("Clear pair") : qsTr("Add pair")
    }

    SilicaListView {
        id: list

        width: parent.width
        height: parent.height - dialogHeader.height
        anchors.top: dialogHeader.bottom
        clip: true

        VerticalScrollDecorator { }

        model: parameterList

        delegate: MouseArea {
            id: parameterItem

            readonly property color highlightedColor: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
            readonly property color pressedColor: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity/2)
            readonly property bool highlighted: dataTable === pairSecondTable
            readonly property bool down: pressed && containsMouse

            enabled: dataTable !== pairFirstTable
            height: Theme.itemSizeMedium
            width: list.width

            onClicked: {
                // toggle logic
                if (pairSecondTable === dataTable) {
                    pairSecondTable = ""
                } else {
                    pairSecondTable = dataTable
                }
            }

            Rectangle {
                anchors.fill: parent
                color: parent.pressed ? pressedColor : parent.highlighted ? highlightedColor : "transparent"
                Behavior on color { ColorAnimation { duration: 50 } }
            }

            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    text: parName
                    width: parent.width
                    truncationMode: TruncationMode.Fade
                    color: parameterItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                    opacity: parameterItem.enabled ? 1.0 : 0.6
                }

                Label {
                    text: parDescription
                    width: parent.width
                    truncationMode: TruncationMode.Fade
                    font.pixelSize: Theme.fontSizeSmall
                    color: parameterItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                    visible: text !== ""
                }
            }
        }
    }
}
