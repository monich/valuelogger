import QtQuick 2.0
import Sailfish.Silica 1.0
import "../js/debug.js" as Debug

Dialog {
    id: newParamaterPage

    canAccept: false

    property string parameterName
    property string parameterDescription
    property string plotColor: plotColors[0]
    property string pageTitle: qsTr("Add")

    onDone: {
        Debug.log("closing:", result)
        if (result === DialogResult.Accepted) {
            parameterName = parNameField.text
            parameterDescription = parDescField.text
            Debug.log("color set to", plotColor)
        }
    }


    SilicaFlickable {
        id: flick

        anchors.fill: parent
        contentHeight: col.height
        width: parent.width

        VerticalScrollDecorator { flickable: flick }

        Column {
            id: col

            spacing: Theme.paddingLarge
            width: newParamaterPage.width

            DialogHeader {
                id: pageHeader
                acceptText: pageTitle + qsTr(" parameter")
                cancelText: qsTr("Cancel")
            }

            TextField {
                id: parNameField
                focus: true
                width: parent.width
                label: qsTr("Parameter name")
                text: parameterName
                placeholderText: qsTr("Enter parameter name here")
                onTextChanged: newParamaterPage.canAccept = text.length > 0
                EnterKey.enabled: text.length > 0
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: parDescField.focus = true
            }

            TextField {
                id: parDescField
                width: parent.width
                label: qsTr("Description")
                text: parameterDescription
                placeholderText: qsTr("Enter short description here")
                EnterKey.enabled: true
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: parNameField.focus = true
            }

            SectionHeader {
                text: qsTr("Plot color")
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 50

                Rectangle {
                    id: plotColorLegend
                    height: 50
                    width: 50
                    color: plotColor
                }

                Button {
                    text: qsTr("Change")
                    anchors.verticalCenter: plotColorLegend.verticalCenter
                    onClicked: {
                        var dialog = pageStack.push("Sailfish.Silica.ColorPickerDialog", { "colors": plotColors })
                        dialog.accepted.connect(function() {
                            Debug.log("Changed color to", dialog.color)
                            plotColorLegend.color = dialog.color
                            plotColor = dialog.color
                        })
                    }
                }
            }
        }
    }
}
