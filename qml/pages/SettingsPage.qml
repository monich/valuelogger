import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

Page {
    SilicaListView {
        id: scroller

        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.StrictlyEnforceRange
        flickDeceleration: maximumFlickVelocity
        clip: true
        anchors.fill: parent
        model: [ settingsViewComponent, aboutViewComponent ]
        delegate: Loader {
            width: scroller.width
            height: scroller.height
            sourceComponent: modelData
        }

        HorizontalScrollDecorator { flickable: scroller }
    }

    Component {
        id: settingsViewComponent

        SettingsView {
            anchors.fill: parent
        }
    }

    Component {
        id: aboutViewComponent

        AboutView {
            anchors.fill: parent
        }
    }
}
