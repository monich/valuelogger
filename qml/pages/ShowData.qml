import QtQuick 2.0
import Sailfish.Silica 1.0

import "../js/debug.js" as Debug

Page {
    id: showDataPage

    property var dataList : []
    property string parName : "Name goes here"
    property string parDescription : "Description goes here"
    property string dataTable : "Data table name here"

    PageHeader {
        id: pageHeader

        title: parName
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
                        onClicked: editData()
                    }

                    MenuItem {
                        text: qsTr("Remove")
                        onClicked: remove()
                    }
                }
            }

            ListView.onRemove: animateRemoval(dataItem)

            function remove() {
                Debug.log("Deleting...")
                remorseAction(qsTr("Deleting"), function() {
                    Logger.deleteData(dataTable, key)
                    dataListView.model.remove(index)
                }, 2500 )
            }

            function editData() {
                var editDialog = pageStack.push(Qt.resolvedUrl("AddValue.qml"), {
                    "allowedOrientations": allowedOrientations,
                    "parameterName": parName,
                    "parameterDescription": parDescription,
                    "value": value,
                    "annotation": annotation,
                    "nowDate": Qt.formatDateTime(new Date(timestamp.trim()), "yyyy-MM-dd"),
                    "nowTime": Qt.formatDateTime(new Date(timestamp.trim()), "h:mm:ss")
                })

                editDialog.accepted.connect( function() {
                    Debug.log("dialog accepted")
                    Debug.log(" value is", editDialog.value)
                    Debug.log(" annotation is", editDialog.annotation)
                    Debug.log(" date is", editDialog.nowDate)
                    Debug.log(" time is", editDialog.nowTime)

                    dataListView.model.setProperty(index, "value", editDialog.value)
                    dataListView.model.setProperty(index, "annotation", editDialog.annotation)
                    dataListView.model.setProperty(index, "timestamp", (editDialog.nowDate + " " + editDialog.nowTime))

                    Logger.addData(dataTable, key, editDialog.value, editDialog.annotation, (editDialog.nowDate + " " + editDialog.nowTime))
                })
            }

            Row {
                id: itemRow

                anchors.verticalCenter: parent.verticalCenter
                x: Theme.horizontalPageMargin
                height: Theme.itemSizeMedium
                spacing: Theme.paddingMedium

                Column {
                    width: dataItem.width - 2 * parent.x - valueLabel.width - parent.spacing
                    anchors.verticalCenter: parent.verticalCenter

                    Label {
                        text: timestamp
                        width: parent.width
                        truncationMode: TruncationMode.Fade
                    }

                    Label {
                        text: annotation
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
                    text: value
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: Theme.fontSizeExtraLarge
                }
            }
        }
    }
}
