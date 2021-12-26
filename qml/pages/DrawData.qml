import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

Page {
    property alias parInfo: plot.parInfoModel

    backNavigation: !plot.dragging

    Component.onCompleted: plot.enableSizeTracking()

    PageHeader {
        id: ph
        title: parInfo.length === 1 ? parInfo[0].name : qsTr("Plot")
    }

    LinePlot {
        id: plot
        x: Theme.horizontalPageMargin
        width: parent.width - 2 * x
        anchors {
            top: ph.bottom
            bottom: parent.bottom
        }
    }
}
