import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    id: cover

    Image {
        id: icon

        y: x
        width: Math.floor(parent.width * 0.56) & (-2)
        source: Qt.resolvedUrl("../images/harbour-valuelogger.svg")
        anchors.horizontalCenter: parent.horizontalCenter
        sourceSize.width: width
    }

    Label {
        id: label

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: icon.bottom
            bottom: parent.bottom
            bottomMargin: (Theme.itemSizeSmall + Theme.iconSizeSmall)/2/cover.parent.scale
        }
        verticalAlignment: Text.AlignVCenter
        truncationMode: TruncationMode.Fade
        minimumPixelSize: Theme.fontSizeTiny
        fontSizeMode: Text.Fit
        color: Theme.highlightColor
        text: par ? par : "Value logger"
        readonly property string par: (lastDataAddedIndex) >= 0 ? parameterList.get(lastDataAddedIndex).parName : ""
    }

    CoverActionList {
        enabled: lastDataAddedIndex >= 0

        CoverAction {
            iconSource: coverIconLeft
            onTriggered: coverLeftClicked()
        }
        CoverAction {
            iconSource: coverIconRight
            onTriggered: coverRightClicked()
        }
    }
}
