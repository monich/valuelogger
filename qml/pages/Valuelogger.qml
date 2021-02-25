import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"
import "../js/debug.js" as Debug

Page {
    id: mainPage

    readonly property real fullHeight: isPortrait ? Screen.height : Screen.width

    function addParameter(index, oldParName, oldParDesc, oldPlotColor) {
        var dialog = pageStack.push(Qt.resolvedUrl("NewParameter.qml"), {
            "allowedOrientations": allowedOrientations
        })

        dialog.accepted.connect(function() {
            Debug.log("dialog accepted")
            Debug.log(dialog.parameterName)
            Debug.log(dialog.parameterDescription)
            Debug.log(dialog.plotColor)

            var datatable = logger.addParameterEntry("", dialog.parameterName, dialog.parameterDescription, true, dialog.plotColor, "")

            parameterList.append({"parName": dialog.parameterName,
                                  "parDescription": dialog.parameterDescription,
                                  "visualize": true,
                                  "plotcolor": logger.colorToString(dialog.plotColor),
                                  "dataTable": datatable,
                                  "pairedTable": "",
                                  "visualizeChanged": false})
        })
    }

    Messagebox { id: messagebox  }

    SilicaFlickable {
        width: parent.width
        height: fullHeight
        contentHeight: height

        PullDownMenu {
            MenuItem {
                text: qsTr("About...")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"), {
                    "allowedOrientations": allowedOrientations,
                    "version": logger.version,
                    "name": "Value Logger",
                    "imagelocation": Qt.resolvedUrl("../images/harbour-valuelogger.svg")
                })
            }

            MenuItem {
                text: qsTr("Export to CSV")
                onClicked: messagebox.showMessage(qsTr("Exported to:") + "<br>" + logger.exportToCSV(), 2500)
            }

            MenuItem {
                text: qsTr("Add new parameter")
                onClicked: addParameter()
            }
        }

        PageHeader {
            id: header

            title: "Value Logger"
        }

        SilicaListView {
            id: parameters

            width: parent.width
            clip: true

            model: parameterList

            anchors {
                top: header.bottom
                bottom: plotButton.top
                bottomMargin: Theme.paddingLarge
            }

            delegate: ListItem {
                id: parameterItem

                contentHeight: Theme.itemSizeMedium
                menu: Component {
                    ContextMenu {
                        MenuItem {
                            text: qsTr("Show raw data")
                            onClicked: {
                                var tmp = logger.readData(dataTable)

                                dataList.clear()

                                for (var i=0; i<tmp.length; i++) {
                                    var row = tmp[i]
                                    var timestamp = row["timestamp"]
                                    var value = row["value"]
                                    Debug.log(i, "=", timestamp, "=", value)
                                    dataList.append({
                                        "key": row["key"],
                                        "value": value,
                                        "timestamp": timestamp,
                                        "annotation": row["annotation"]
                                    })
                                }
                                pageStack.push(Qt.resolvedUrl("ShowData.qml"), {
                                    "allowedOrientations": allowedOrientations,
                                    "parName": parName,
                                    "parDescription": parDescription,
                                    "dataList": dataList,
                                    "dataTable": dataTable
                                })
                            }
                        }

                        MenuItem {
                            text: qsTr("Edit")
                            onClicked: editParameter()
                        }

                        MenuItem {
                            text: qsTr("Pair")
                            onClicked: pairParameter()
                        }

                        MenuItem {
                            text: qsTr("Remove")
                            onClicked: remove()
                        }
                    }
                }

                ListView.onRemove: animateRemoval(parameterItem)

                function remove() {
                    remorseAction(qsTr("Deleting"), function() {
                        // remove this from parameters where it is paired to
                        for (var i=0; i<parameters.model.count; i++)
                        {
                            var tmp = parameters.model.get(i)
                            if (tmp.pairedTable === dataTable)
                            {
                                parameters.model.setProperty(i, "pairedTable", "")
                                logger.setPairedTable(tmp.dataTable, "")
                            }
                        }

                        logger.deleteParameterEntry(parName, dataTable)
                        parameters.model.remove(index)
                        lastDataAddedIndex = -1
                    })
                }

                function editParameter() {
                    var dialog = pageStack.push(Qt.resolvedUrl("NewParameter.qml"), {
                        "allowedOrientations": allowedOrientations,
                        "parameterName": parName,
                        "parameterDescription": parDescription,
                        "plotColor": plotcolor,
                        "pageTitle": qsTr("Edit")
                    })

                    dialog.accepted.connect(function() {
                        Debug.log("EDIT dialog accepted")
                        Debug.log(dialog.parameterName)
                        Debug.log(dialog.parameterDescription)
                        Debug.log(dialog.plotColor)

                        logger.addParameterEntry(dataTable, dialog.parameterName, dialog.parameterDescription, visualize, dialog.plotColor, pairedTable)

                        parameters.model.setProperty(index, "parName", dialog.parameterName)
                        parameters.model.setProperty(index, "parDescription", dialog.parameterDescription)
                        parameters.model.setProperty(index, "plotcolor", logger.colorToString(dialog.plotColor))
                    })
                }

                function pairParameter() {
                    var dialog = pageStack.push(Qt.resolvedUrl("AddPair.qml"), {
                        "allowedOrientations": allowedOrientations,
                        "pairFirstTable": dataTable,
                        "pairSecondTable": pairedTable
                    })

                    dialog.accepted.connect(function() {
                        Debug.log("Add pair dialog accepted")
                        Debug.log(dialog.pairSecondTable)
                        logger.setPairedTable(dataTable, dialog.pairSecondTable)
                        parameters.model.setProperty(index, "pairedTable", dialog.pairSecondTable)
                    })
                }

                Item {
                    width: parent.width - Theme.horizontalPageMargin
                    height: parent.height

                    Switch {
                        id: parSwitch

                        anchors.verticalCenter: parent.verticalCenter
                        checked: visualize
                        onCheckedChanged: {
                            parameterList.setProperty(index, "visualize", checked)
                            parameterList.setProperty(index, "visualizeChanged", true)
                        }
                    }

                    Column {
                        anchors {
                            left: parSwitch.right
                            right: rightArea.left
                            rightMargin: Theme.paddingLarge
                            verticalCenter: parent.verticalCenter
                        }

                        Label {
                            width: parent.width
                            color: parameterItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                            truncationMode: TruncationMode.Fade
                            text: parName
                        }
                        Label {
                            width: parent.width
                            font.pixelSize: Theme.fontSizeSmall
                            truncationMode: TruncationMode.Fade
                            color: parameterItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                            text: parDescription
                            visible: text !== ""
                        }
                    }

                    Row {
                        id: rightArea

                        height: parent.height
                        spacing: Theme.paddingLarge
                        anchors.right: parent.right

                        Image {
                            id: pairIcon
                            source: "image://theme/icon-m-link"
                            anchors.verticalCenter: parent.verticalCenter
                            sourceSize: Qt.size(Theme.iconSizeMedium, Theme.iconSizeMedium)
                            visible: pairedTable !== ""
                        }

                        IconButton {
                            id: addValueButton
                            anchors.verticalCenter: parent.verticalCenter
                            icon.source: "image://theme/icon-m-add"
                            onClicked: {
                                Debug.log("clicked add value button")

                                lastDataAddedIndex = index

                                var dialog = pageStack.push(Qt.resolvedUrl("AddValue.qml"), {
                                    "allowedOrientations": allowedOrientations,
                                    "parameterName": parName,
                                    "parameterDescription": parDescription
                                })

                                if (pairedTable !== "") {
                                    Debug.log("this is a paired parameter")
                                    var paired_parName = "ERROR"
                                    var paired_parDescription = "ERROR"

                                    for (var i=0; i<parameterList.count; i++) {
                                        var tmp = parameterList.get(i)
                                        if (tmp.dataTable === pairedTable) {
                                            paired_parName = tmp.parName
                                            paired_parDescription = tmp.parDescription
                                            Debug.log("found ", tmp.parName, tmp.parDescription)
                                            break
                                        }
                                    }

                                    var pairdialog = pageStack.pushAttached(Qt.resolvedUrl("AddValue.qml"), {
                                        "allowedOrientations": allowedOrientations,
                                        "nowDate": dialog.nowDate,
                                        "nowTime": dialog.nowTime,
                                        "parameterName": paired_parName,
                                        "parameterDescription": paired_parDescription,
                                        "paired": true
                                    })

                                    pairdialog.accepted.connect(function() {
                                        Debug.log("paired dialog accepted")
                                        Debug.log(" value is", pairdialog.value)
                                        Debug.log(" annotation is", pairdialog.annotation)
                                        Debug.log(" date is", pairdialog.nowDate)
                                        Debug.log(" time is", pairdialog.nowTime)

                                        logger.addData(pairedTable, "", pairdialog.value, pairdialog.annotation, pairdialog.nowDate + " " + pairdialog.nowTime)
                                    })
                                }

                                dialog.accepted.connect(function() {
                                    Debug.log("dialog accepted")
                                    Debug.log(" value is", dialog.value)
                                    Debug.log(" annotation is", dialog.annotation)
                                    Debug.log(" date is", dialog.nowDate)
                                    Debug.log(" time is", dialog.nowTime)

                                    logger.addData(dataTable, "", dialog.value, dialog.annotation, dialog.nowDate + " " + dialog.nowTime)
                                })
                            }
                        }
                    }
                }

                ListModel { id: dataList }
            }

            VerticalScrollDecorator { flickable: parameters }
        }


        Button {
            id: plotButton

            anchors {
                bottom: parent.bottom
                bottomMargin: Theme.paddingLarge
                horizontalCenter: parent.horizontalCenter
            }

            text: qsTr("Plot selected")
            enabled: parameterList.count > 0

            onClicked: {
                Debug.log("there is", parameterList.count, " items in list.")

                var l = []
                parInfo.clear()

                for (var a=0; a<parameterList.count; a++) {
                    /* Save changes if visualize touched */
                    var par = parameterList.get(a)
                    if (par.visualizeChanged) {
                        logger.addParameterEntry(par.dataTable,
                                                 par.parName,
                                                 par.parDescription,
                                                 par.visualize,
                                                 par.plotcolor,
                                                 par.pairedTable)
                    }

                    if (par.visualize) {
                        Debug.log("showing data from", par.parName)
                        parInfo.append({"name": par.parName, "plotcolor": par.plotcolor})
                        l.push(logger.readData(par.dataTable))
                    }
                }

                if (l.length > 0 && l.length < 10) {
                    pageStack.push(Qt.resolvedUrl("DrawData.qml"), {
                        "allowedOrientations": allowedOrientations,
                        "dataList": l,
                        "parInfo": parInfo
                    })
                } else {
                    Debug.log("ERROR: None or too many plots selected")
                }
            }
        }
    }
}
