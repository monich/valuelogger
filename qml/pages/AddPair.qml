import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

import "../components"

Dialog {
    property alias pairFirstTable: filterModel.ignoreDataTable
    property string pairSecondTable
    property string title

    property bool _hadSecondTable
    readonly property bool _haveSecondTable: pairSecondTable !== ""

    canAccept: _haveSecondTable || _hadSecondTable

    Component.onCompleted: _hadSecondTable = (pairSecondTable !== "")

    DialogHeader {
        id: dialogHeader

        spacing: 0
        acceptText: _haveSecondTable ? qsTr("Pair") :
            _hadSecondTable ? qsTr("Clear pair") : ""
    }

    SilicaListView {
        id: list

        width: parent.width
        anchors {
            top: dialogHeader.bottom
            bottom: parent.bottom
        }
        clip: true

        model: PairModel {
            id: filterModel

            sourceModel: Logger
        }

        header: Column {
            id: headerItem

            x: Theme.horizontalPageMargin
            width: parent.width - x - Theme.horizontalPageMargin

            VerticalGap { height: Theme.paddingMedium }

            Row {
                spacing: Theme.paddingLarge

                HighlightIcon {
                    id: pairIcon
                    source: "image://theme/icon-m-link"
                    anchors.verticalCenter: parent.verticalCenter
                    sourceSize: Qt.size(Theme.iconSizeMedium, Theme.iconSizeMedium)
                }

                Label {
                    text: title
                    width: headerItem.width - pairIcon.width - parent.spacing
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: Theme.fontSizeExtraLarge
                    truncationMode: TruncationMode.Fade
                    color: Theme.highlightColor
                }
            }

            VerticalGap { height: Theme.paddingMedium }
        }

        delegate: BackgroundItem {
            id: parameterItem

            readonly property bool paired: model.datatable === pairSecondTable

            highlighted: down || paired
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

            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    text: model.name
                    width: parent.width
                    truncationMode: TruncationMode.Fade
                    color: parameterItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                }

                Label {
                    text: model.description
                    width: parent.width
                    truncationMode: TruncationMode.Fade
                    font {
                        pixelSize: Theme.fontSizeSmall
                        italic: true
                    }
                    color: parameterItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                    visible: text !== ""
                }
            }
        }

        VerticalScrollDecorator { }
    }
}
