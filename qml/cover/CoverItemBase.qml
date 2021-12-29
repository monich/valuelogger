import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

Item {
    /* API */

    property real coverScale: 1.0
    property bool showGraph: true
    property color graphColor
    property var graphModel
    property alias title: label.text

    signal updateGraph()

    /* Internal API */

    property real graphBottomY
    property bool showIcon: !showGraph

    readonly property bool darkOnLight: 'colorScheme' in Theme && Theme.colorScheme === Theme.DarkOnLight
    readonly property int thinLine: Math.max(1, Math.floor(Theme.paddingSmall/4))
    readonly property int thickLine: Math.max(2, Math.floor(Theme.paddingSmall/2))

    /* Implementation */

    Item {
        id: iconItem

        readonly property int padding: Theme.paddingSmall

        y: x
        width: parent.width / 2 + 2 * padding
        height: width
        anchors.horizontalCenter: parent.horizontalCenter
        visible: showIcon

        HighlightIcon {
            source: showIcon ? "../images/icon-background.svg" : ""
            highlightColor: Theme.primaryColor
            anchors.centerIn: parent
            sourceSize.width: icon.width + parent.padding
            sourceSize.height: icon.height + parent.padding
            opacity: 0.2
        }

        Image {
            id: icon

            source: showIcon ? "../images/harbour-valuelogger2.svg" : ""
            anchors.centerIn: parent
            sourceSize.width: parent.width - 2 * parent.padding
        }
    }

    Label {
        id: label

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: showIcon ? (iconItem.y + iconItem.height) : graphBottomY
            bottom: parent.bottom
            bottomMargin: (Theme.itemSizeSmall + Theme.iconSizeSmall)/2/coverScale
        }
        width: Math.min(implicitWidth, parent.width - 2 * Theme.paddingLarge)
        verticalAlignment: Text.AlignVCenter
        truncationMode: TruncationMode.Fade
        minimumPixelSize: Theme.fontSizeExtraSmall
        fontSizeMode: Text.Fit
        color: Theme.highlightColor
    }
}
