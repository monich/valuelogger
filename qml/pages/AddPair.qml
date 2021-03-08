import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

Dialog {
    property string pairFirstTable
    property string pairSecondTable

    property bool _hadSecondTable
    readonly property bool _haveSecondTable: pairSecondTable !== ""

    canAccept: _haveSecondTable || _hadSecondTable

    Component.onCompleted: _hadSecondTable = (pairSecondTable !== "")

    DialogHeader {
        id: dialogHeader

        acceptText: _haveSecondTable ? qsTr("Add pair") :
            _hadSecondTable ? qsTr("Clear pair") : ""
    }

    SilicaListView {
        id: list

        width: parent.width
        height: parent.height - dialogHeader.height
        anchors.top: dialogHeader.bottom
        clip: true

        VerticalScrollDecorator { }

        model: Logger

        delegate: MouseArea {
            id: parameterItem

            readonly property color highlightedColor: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
            readonly property color pressedColor: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity/2)
            readonly property bool highlighted: model.datatable === pairSecondTable
            readonly property bool down: pressed && containsMouse

            enabled: model.datatable !== pairFirstTable
            height: Theme.itemSizeMedium
            width: list.width

            onClicked: {
                // toggle logic
                if (pairSecondTable === model.datatable) {
                    pairSecondTable = ""
                } else {
                    pairSecondTable = model.datatable
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
                    text: model.name
                    width: parent.width
                    truncationMode: TruncationMode.Fade
                    color: parameterItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                    opacity: parameterItem.enabled ? 1.0 : 0.6
                }

                Label {
                    text: model.description
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
