/*
    Copyright (C) 2014-2015 Kimmo Lindholm
    Copyright (C) 2021 Slava Monich
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    property string name
    property string version
    property string imagelocation

    SilicaFlickable  {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
            spacing: Theme.paddingLarge

            PageHeader { title: name }

            Image {
                visible: imagelocation.length > 0
                anchors.horizontalCenter: parent.horizontalCenter
                sourceSize.width: Theme.itemSizeHuge
                source: imagelocation
            }

            Label {
                width: parent.width
                text: qsTr("Version: %1").arg(version)
                minimumPixelSize: Theme.fontSizeTiny
                fontSizeMode: Text.Fit
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                width: parent.width
                text: "© 2014-2015 kimmoli\n© 2021 Slava Monich"
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
