import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

Page {
    property alias parInfo: plot.parInfoModel

    backNavigation: !plot.dragging

    Component.onCompleted: plot.enableSizeTracking()

    PageHeader {
        id: header

        title: parInfo.length === 1 ? parInfo[0].name : qsTr("Plot")
        description: parInfo.length === 1 ? parInfo[0].description : ""
    }

    LinePlot {
        id: plot

        x: Theme.horizontalPageMargin
        width: parent.width - 2 * x
        anchors {
            top: header.bottom
            bottom: parent.bottom
        }
    }
}
