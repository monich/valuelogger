import QtQuick 2.0
import Sailfish.Silica 1.0

import "../js/debug.js" as Debug
import "../components"

Dialog {
    readonly property color selectedColor: sample.color
    property color initialColor

    canAccept: hexText.acceptableInput
    forwardNavigation: !colorPanel.pressed
    backNavigation: !colorPanel.pressed

    DialogHeader {
        id: header

        acceptText: forwardNavigation ? qsTr("Add color") : ""
    }

    // Otherwise width is changing with a delay, causing visible layout changes
    onIsLandscapeChanged: width = isLandscape ? Screen.height : Screen.width

    SilicaFlickable {
        clip: true
        interactive: !colorPanel.pressed
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }

        ColorPanel {
            id: colorPanel

            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
            anchors {
                top: parent.top
                bottom: toolPanel.top
                bottomMargin: Theme.paddingSmall
            }
            brightness: brightnessSlider.sliderValue
            onTapped: hueSlider.value = h
        }

        Column {
            id: toolPanel

            width: parent.width
            anchors.bottom: parent.bottom

            Slider {
                id: hueSlider

                width: parent.width
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
                minimumValue: 0
                maximumValue: 1
                value: 1
                label: qsTr("Color")
                opacity: (y + parent.y >= 0) ? 1 : 0
                onSliderValueChanged: hexText.updateText()
            }

            Slider {
                id: brightnessSlider

                width: parent.width
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
                label: qsTr("Brightness")
                minimumValue: 0
                maximumValue: 1
                value: 1
                opacity: (y + parent.y >= 0) ? 1 : 0
                onSliderValueChanged: hexText.updateText()
            }

            Item {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                height: hexText.height
                Row {
                    spacing: Theme.paddingSmall

                    Label {
                        id: hexLabel

                        y: hexText.textTopMargin
                        text: "#"
                        font.pixelSize: Theme.fontSizeLarge
                    }

                    Item {
                        readonly property int maxWidth: toolPanel.width - 2 * Theme.horizontalPageMargin - hexLabel.width - parent.spacing - Theme.paddingLarge - sample.width
                        width: Math.min(Math.max(textLabelTemplate.implicitWidth, Theme.itemSizeHuge), maxWidth)
                        height: hexText.height

                        Label {
                            id: textLabelTemplate

                            // Same text as hexText.label
                            text: qsTr("Hex notation")
                            font.pixelSize: Theme.fontSizeSmall
                            opacity: 0
                        }

                        TextField {
                            id: hexText

                            property int ignoreTextUpdates // to avoid binding loops
                            property color tmpColor

                            font.pixelSize: Theme.fontSizeLarge
                            width: parent.width
                            textLeftMargin: 0
                            textRightMargin: 0
                            label: qsTr("Hex notation")
                            text: initialColor.toString().substr(1)
                            validator: RegExpValidator { regExp: /^[0-9a-fA-F]{6}$/ }
                            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                            EnterKey.iconSource: "image://theme/icon-m-enter-close"
                            EnterKey.onClicked: hexText.focus = false

                            onTextChanged: {
                                if (!ignoreTextUpdates && acceptableInput) {
                                    tmpColor = "#" + text
                                    Debug.log(tmpColor)
                                    ignoreTextUpdates++
                                    brightnessSlider.value = colorPanel.getV(tmpColor)
                                    hueSlider.value = colorPanel.getH(tmpColor)
                                    ignoreTextUpdates--
                                }
                            }

                            onActiveFocusChanged: {
                                if (!activeFocus && !acceptableInput) {
                                    updateText()
                                }
                            }

                            function updateText() {
                                if (!ignoreTextUpdates) {
                                    ignoreTextUpdates++
                                    var s = colorPanel.getColor(hueSlider.sliderValue).toString()
                                    text = (s.length > 0 && s.charAt(0) === '#') ? s.substr(1) : s
                                    ignoreTextUpdates--
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    id: sample

                    y: Theme.paddingLarge
                    width: 2 * height
                    height: hexText.height - 2 * Theme.paddingLarge
                    anchors.right: parent.right
                    color: "#" + hexText.text
                    visible: hexText.acceptableInput
                }
            }
        }
    }
}
