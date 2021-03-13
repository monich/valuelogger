import QtQuick 2.0
import Sailfish.Silica 1.0

import "../js/debug.js" as Debug
import "../components"

Dialog {
    id: addValuePage

    canAccept: valueField.text.length > 0

    property string parameterName
    property string parameterDescription
    property string pageTitle /* Add or Edit*/
    property string value
    property alias annotation: annotationField.text
    property string nowDate
    property string nowTime
    property bool paired

    Component.onCompleted: {
        // Check are we adding new, or editing existing one
        if (nowDate === "") {
            var tmp = new Date()
            nowDate = Qt.formatDateTime(tmp, "yyyy-MM-dd")
            nowTime = Qt.formatDateTime(tmp, "hh:mm:ss")
            pageTitle = qsTr("Add")
        } else {
            if (nowTime == "") nowTime = "00:00:00"
            valueField.text = value
            pageTitle = qsTr("Edit")
        }
        valueField.textChanged.connect(function() { value = valueField.text.replace(",",".") })
    }

    DialogHeader {
        id: dialogHeader

        acceptText: pageTitle + qsTr(" value")
        cancelText: qsTr("Cancel")
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
            spacing: Theme.paddingLarge

            Row {
                x: Theme.horizontalPageMargin

                Image {
                    id: pairIcon
                    source: "image://theme/icon-m-link"
                    anchors.verticalCenter: parent.verticalCenter
                    visible: paired
                }

                Column {
                    width: col.width - 2 * parent.x - x
                    anchors.verticalCenter: parent.verticalCenter

                    Label {
                        width: parent.width
                        text: parameterName
                        font.pixelSize: Theme.fontSizeExtraLarge
                        color: Theme.highlightColor
                        truncationMode: TruncationMode.Fade
                    }

                    Label {
                        width: parent.width
                        text: parameterDescription
                        color: Theme.secondaryHighlightColor
                        truncationMode: TruncationMode.Fade
                        visible: text !== ""
                    }
                }
            }

            Row {
                x: Theme.horizontalPageMargin

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: col.width - modifyDateButton.width - modifyTimeButton.width - 2 * parent.x

                    Label {
                        id: dateNow

                        width: parent.width
                        text: nowDate + " " + nowTime
                    }

                    Label {
                        width: parent.width
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                        text: qsTr("Timestamp")
                    }
                }

                MouseArea {
                    id: modifyDateButton

                    width: Theme.iconSizeMedium
                    height: width
                    anchors.verticalCenter: parent.verticalCenter

                    readonly property bool showPress: (pressed && containsMouse) || modifyDateButtonPressTimer.running

                    HighlightIcon {
                        source: "../images/calendar-icon.svg"
                        highlightColor: parent.showPress ? Theme.highlightColor : Theme.primaryColor
                        anchors.centerIn: parent
                        sourceSize.height: Theme.iconSizeMedium
                    }

                    onPressedChanged: {
                        if (pressed) {
                            modifyDateButtonPressTimer.start()
                        }
                    }

                    onCanceled: modifyDateButtonPressTimer.stop()

                    onClicked: {
                        Debug.log("modifyDateButton clicked")

                        var dialogDate = pageStack.push(pickerDate, { date: new Date(nowDate) })
                        dialogDate.accepted.connect(function() {
                            Debug.log("You chose:", dialogDate.dateText)
                            // use date, as dateText return varies
                            nowDate = Qt.formatDateTime(new Date(dialogDate.date), "yyyy-MM-dd")
                        })
                    }

                    Timer {
                        id: modifyDateButtonPressTimer
                        interval: 50
                    }

                    Component {
                        id: pickerDate
                        DatePickerDialog {}
                    }
                }

                IconButton {
                    id: modifyTimeButton

                    anchors.verticalCenter: parent.verticalCenter
                    icon.source: "image://theme/icon-m-time-date"

                    onClicked: {
                        var d = new Date(dateNow.text)
                        var h = Qt.formatDateTime(d, "hh")
                        var m = Qt.formatDateTime(d, "mm")

                        Debug.log("modifyTimeButton clicked")
                        Debug.log("hour", h)
                        Debug.log("minute", m)

                        var dialogTime = pageStack.push(pickerTime, {hour: h, minute: m})
                        dialogTime.accepted.connect(function() {
                            Debug.log("You chose:", dialogTime.timeText)
                            var tt = dialogTime.timeText + ":00"
                            if (dialogTime.hour.length < 2) tt = "0" + tt
                            nowTime = tt
                        })
                    }

                    Component {
                        id: pickerTime
                        TimePickerDialog {}
                    }
                }
            }

            TextField {
                id: valueField

                focus: true
                width: parent.width
                label: qsTr("Value")
                font.pixelSize: Theme.fontSizeExtraLarge
                placeholderText: qsTr("Enter new value here")
                inputMethodHints: Qt.ImhDigitsOnly
                validator: RegExpValidator { regExp: /-?\d+([,|\.]?\d+)?/ }
                EnterKey.enabled: text.length > 0
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: annotationField.focus = true
            }

            TextField {
                id: annotationField

                focus: false
                width: parent.width
                label: qsTr("Annotation")
                font.pixelSize: Theme.fontSizeExtraLarge
                placeholderText: qsTr("Enter annotation here")
                EnterKey.enabled: valueField.text.length > 0
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: addValuePage.accept()
                onFocusChanged: if (focus) flick.scrollToBottom()
            }
        }
    }
}
