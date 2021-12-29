import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

import "../js/debug.js" as Debug

Page {
    id: showDataPage

    property string parName : "Name goes here"
    property string parDescription : "Description goes here"
    property alias dataTable: dataModel.dataTable

    SilicaFlickable {
        anchors.fill: parent
        ViewPlaceholder {
            enabled: dataListView.count === 0
            text: qsTr("No data.")
        }
    }

    DataTableModel {
        id: dataModel
    }

    PageHeader {
        id: pageHeader

        title: parName
        description: parDescription
    }

    SilicaListView {
        id: dataListView

        VerticalScrollDecorator { flickable: dataListView }

        model: dataModel

        width: parent.width
        height: parent.height - pageHeader.height
        anchors.top: pageHeader.bottom
        clip: true

        delegate: ListItem {
            id: dataItem

            width: dataListView.width
            menu: Component {
                ContextMenu {
                    MenuItem {
                        text: qsTr("Edit")
                        onClicked: dataItem.editData()
                    }
                    MenuItem {
                        text: qsTr("Remove")
                        onClicked: dataItem.removeEntry()
                    }
                }
            }

            ListView.onRemove: animateRemoval(dataItem)

            function removeEntry() {
                Debug.log("preparing to delete", model.key, "...")
                remorseAction(qsTr("Deleting"), removalConfirmed, 2500)
            }

            function removalConfirmed() {
                Debug.log("deleting", model.key, "at", model.index)
                dataModel.deleteRow(model.index)
                Logger.tableUpdated(dataTable)
            }

            function editData() {
                var editDialog = pageStack.push(Qt.resolvedUrl("AddValue.qml"), {
                    "allowedOrientations": allowedOrientations,
                    "parameterName": parName,
                    "parameterDescription": parDescription,
                    "value": valueLabel.text,
                    "annotation": annotationLabel.text,
                    "nowDate": Qt.formatDateTime(model.timestamp, "yyyy-MM-dd"),
                    "nowTime": Qt.formatDateTime(model.timestamp, "hh:mm:ss")
                })

                editDialog.accepted.connect( function() {
                    Debug.log("dialog accepted")

                    var timestamp = editDialog.nowDate + " " + editDialog.nowTime
                    Debug.log(" value", editDialog.value)
                    Debug.log(" annotation", editDialog.annotation)
                    Debug.log(" time", timestamp)

                    dataModel.updateRow(model.index, editDialog.value, editDialog.annotation, timestamp)
                    Logger.tableUpdated(dataTable)
                })
            }

            Row {
                x: Theme.horizontalPageMargin
                height: Theme.itemSizeMedium
                spacing: Theme.paddingMedium
                anchors.verticalCenter: parent.verticalCenter

                Column {
                    width: dataItem.width - 2 * parent.x - valueLabel.width - parent.spacing
                    anchors.verticalCenter: parent.verticalCenter

                    Label {
                        id: timestampLabel

                        text: Qt.formatDateTime(model.timestamp, "yyyy-MM-dd hh:mm:ss")
                        width: parent.width
                        color: dataItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                        truncationMode: TruncationMode.Fade
                    }

                    Label {
                        id: annotationLabel

                        text: model.annotation
                        width: parent.width
                        truncationMode: TruncationMode.Fade
                        visible: text !== ""
                        color: dataItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                        font {
                            italic: true
                            pixelSize: Theme.fontSizeSmall
                        }
                    }
                }

                Label {
                    id: valueLabel

                    anchors.verticalCenter: parent.verticalCenter
                    text: model.value
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: Theme.fontSizeExtraLarge
                    color: dataItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                }
            }
        }
    }
}
