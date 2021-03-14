import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

CoverBackground {
    id: cover

    signal addValue()
    signal plotSelected()

    readonly property bool darkOnLight: 'colorScheme' in Theme && Theme.colorScheme === Theme.DarkOnLight

    Image {
        id: icon

        y: x
        width: parent.width / 2
        source: Qt.resolvedUrl("../images/harbour-valuelogger2.svg")
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
        text: Logger.defaultParameterName ? Logger.defaultParameterName : "Value logger"
    }

    CoverActionList {
        enabled: Logger.visualizeCount === 1

        CoverAction {
            iconSource: "image://theme/icon-cover-new"
            onTriggered: addValue()
        }
        CoverAction {
            iconSource: Qt.resolvedUrl("../images/" + (darkOnLight ? "icon-cover-plot-dark.svg" : "icon-cover-plot.svg"))
            onTriggered: cover.plotSelected()
        }
    }

    CoverActionList {
        enabled: Logger.visualizeCount > 1

        CoverAction {
            iconSource: "../icon-cover-plot.png"
            onTriggered: cover.plotSelected()
        }
    }
}
