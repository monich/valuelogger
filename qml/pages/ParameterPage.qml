import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0
import harbour.valuelogger 1.0

import "../js/debug.js" as Debug
import "../components"

Dialog {
    id: newParamaterPage

    canAccept: false

    property string parameterName
    property string parameterDescription
    property string plotColor: plotColors[0]
    property alias pageTitle: dialogHeader.acceptText

    onDone: {
        Debug.log("closing:", result)
        if (result === DialogResult.Accepted) {
            parameterName = parNameField.text
            parameterDescription = parDescField.text
            Debug.log("color set to", plotColor)
        }
    }

    ConfigurationValue {
        id: colorsConfig

        key: Settings.configRoot + "colors"
        defaultValue: [
            "#ffffff", "#ff00ff", "#ff0080",
            "#808080", "#ff8000", "#ffff00",
            "#00ff80", "#00ff00", "#00ffff",
            "#008080", "#8000ff", "#0000ff"
        ]
    }

    DialogHeader {
        id: dialogHeader

        acceptText: qsTr("Add parameter")
    }

    SilicaFlickable {
        id: flick

        clip: true
        contentHeight: col.height
        width: parent.width
        anchors {
            top: dialogHeader.bottom
            bottom: parent.bottom
        }

        VerticalScrollDecorator { flickable: flick }

        Column {
            id: col

            width: parent.width

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

            VerticalGap { }

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

            BackgroundItem {
                id: colorItem

                width: parent.width

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: Theme.paddingLarge
                    x: Theme.horizontalPageMargin

                    Rectangle {
                        id: plotColorLegend

                        width: parent.height
                        height: width
                        color: plotColor
                        anchors.verticalCenter: parent.verticalCenter
                        layer.enabled: colorItem.highlighted
                        layer.effect: PressEffect { source: plotColorLegend }
                    }

                    Label {
                        text: qsTr("Plot color")
                        anchors.verticalCenter: parent.verticalCenter
                        color: colorItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                    }
                }

                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("ColorPicker.qml"), {
                        "allowedOrientations": allowedOrientations,
                        "colors": colorsConfig.value,
                        "color": plotColor,
                    })
                    dialog.colorsChanged.connect(function() {
                        Debug.log(dialog.colors)
                        colorsConfig.value = dialog.colors
                    })
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
