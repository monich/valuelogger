import QtQuick 2.0
import Sailfish.Silica 1.0

Rectangle {
    id: root

    property bool selected
    property string source
    property var dataModel

    signal clicked()

    readonly property bool darkOnLight: 'colorScheme' in Theme && Theme.colorScheme === Theme.DarkOnLight
    readonly property url plotIconSource: Qt.resolvedUrl("../images/" + (darkOnLight ? "icon-cover-plot-dark.svg" : "icon-cover-plot.svg"))
    property alias coverItem: coverItemLoader.item

    width: Theme.coverSizeSmall.width
    height: Theme.coverSizeSmall.height
    color: Theme.rgba(Theme.highlightBackgroundColor, mouseArea.pressed ? Theme.highlightBackgroundOpacity : 0.2)
    radius: Theme.paddingSmall
    border {
        color: Theme.highlightColor
        width: root.selected ? Theme.paddingSmall/2 : 0
    }
    layer.enabled: mouseArea.pressed
    layer.effect: PressEffect {
        source: root
    }

    Loader {
        id: coverItemLoader

        width: Theme.coverSizeLarge.width
        height: Theme.coverSizeLarge.height
        scale: Theme.coverSizeSmall.width/Theme.coverSizeLarge.width
        anchors.centerIn: parent
        source: Qt.resolvedUrl(root.source)
    }

    Binding { target: coverItem;  property: "coverScale"; value: Theme.coverSizeSmall.width/Theme.coverSizeLarge.width }
    Binding { target: coverItem;  property: "graphColor"; value: Theme.highlightBackgroundColor }
    Binding { target: coverItem;  property: "graphModel"; value: dataModel }
    Binding { target: coverItem;  property: "title"; value: qsTr("Example") }

    Grid {
        anchors.bottom: parent.bottom
        columns: 2

        ActionButton {
            width: root.width/2
            source: "image://theme/icon-cover-new"
        }

        ActionButton {
            width: root.width/2
            source: root.plotIconSource
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onClicked: root.clicked()
    }
}
