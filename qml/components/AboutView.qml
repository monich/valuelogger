import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

SilicaFlickable  {
    anchors.fill: parent
    contentHeight: column.height + Theme.paddingLarge

    Column {
        id: column

        width: parent.width
        spacing: Theme.paddingLarge

        PageHeader { title: "Value Logger" }

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            sourceSize.width: Theme.itemSizeHuge
            source: Qt.resolvedUrl("../images/harbour-valuelogger2.svg")
        }

        Label {
            width: parent.width - 2 * Theme.horizontalPageMargin
            text: qsTr("Version: %1").arg(Logger.version)
            minimumPixelSize: Theme.fontSizeTiny
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            width: parent.width - 2 * Theme.horizontalPageMargin
            text: "© 2014-2015 kimmoli\n© 2021 Slava Monich"
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeSmall
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
