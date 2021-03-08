import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

import "../js/debug.js" as Debug

Page {
    id: showDataPage

    property var dataList : []
    property string parName : "Name goes here"
    property string parDescription : "Description goes here"
    property string dataTable : "Data table name here"

    signal deleteData(var key)

    PageHeader {
        id: pageHeader

        title: parName
        description: parDescription
    }

    SilicaListView {
        id: dataListView

        VerticalScrollDecorator { flickable: dataListView }

        model: dataList

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
                Debug.log("deleting", modelData.key ,"...")
                remorseAction(qsTr("Deleting"), function() { showDataPage.deleteData(modelData.key)}, 2500)
            }

            function editData() {
                var editDialog = pageStack.push(Qt.resolvedUrl("AddValue.qml"), {
                    "allowedOrientations": allowedOrientations,
                    "parameterName": parName,
                    "parameterDescription": parDescription,
                    "value": valueLabel.text,
                    "annotation": annotationLabel.text,
                    "nowDate": Qt.formatDateTime(new Date(timestampLabel.text), "yyyy-MM-dd"),
                    "nowTime": Qt.formatDateTime(new Date(timestampLabel.text), "h:mm:ss")
                })

                editDialog.accepted.connect( function() {
                    Debug.log("dialog accepted")

                    var timestamp = editDialog.nowDate + " " + editDialog.nowTime
                    Debug.log(" value", editDialog.value)
                    Debug.log(" annotation", editDialog.annotation)
                    Debug.log(" time", timestamp)

                    Logger.addData(dataTable, modelData.key, editDialog.value, editDialog.annotation, timestamp)

                    /* Break the bindings for now.
                     * TODO: use a real modifiable data model */
                    valueLabel.text = editDialog.value
                    annotationLabel.text = editDialog.annotation
                    timestampLabel.text = timestamp
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

                        text: modelData.timestamp
                        width: parent.width
                        truncationMode: TruncationMode.Fade
                    }

                    Label {
                        id: annotationLabel

                        text: modelData.annotation
                        width: parent.width
                        truncationMode: TruncationMode.Fade
                        visible: text !== ""
                        color: Theme.secondaryColor
                        font {
                            italic: true
                            pixelSize: Theme.fontSizeSmall
                        }
                    }
                }

                Label {
                    id: valueLabel

                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.value
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: Theme.fontSizeExtraLarge
                }
            }
        }
    }

    ViewPlaceholder {
        enabled: dataListView.count === 0
        text: qsTr("Empty")
    }
}
