import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

Dialog {
    id: addValuePage

    canAccept: false

    property string parameterName: "value"
    property string parameterDescription: "value"
    property string pageTitle: "value"  /* Add or Edit*/
    property string value: "value"
    property string annotation: "annotation"
    property string nowDate: "value"
    property string nowTime: "value"
    property bool paired: false

    Component.onCompleted: {
        // Check are we adding new, or editing existing one
        if (nowDate == "value" && nowTime == "value" && value == "value") {
            var tmp = new Date()
            updateDateTime(Qt.formatDateTime(tmp, "yyyy-MM-dd"), Qt.formatDateTime(tmp, "hh:mm:ss"))
            pageTitle = qsTr("Add")
        } else {
            if (nowTime == "") nowTime = "00:00:00"
            updateDateTime(nowDate, nowTime)
            valueField.text = (value == "value") ? "" : value
            annotationField.text = (value == "value") ? "" : annotation
            pageTitle = (value == "value") ? qsTr("Add") : qsTr("Edit")
        }
    }

    function updateDateTime(newDate, newTime) {
        console.log("newdate", newDate, "newtime", newTime)
        nowDate = Qt.formatDateTime(new Date(newDate), "yyyy-MM-dd")
        nowTime = Qt.formatDateTime(new Date(newDate + " " + newTime), "hh:mm:ss")
        console.log("nowdate", nowDate, "nowtime", nowTime)

        dateNow.text = nowDate + " " + nowTime
        console.log("dateNow", dateNow.text)
    }

    onDone: {
        if (result === DialogResult.Accepted) {
            value = valueField.text.replace(",",".")
            annotation = annotationField.text
        }
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
        anchors {
            left: parent.left
            right: parent.right
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
                        text: "unknown"
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
                        console.log("modifyDateButton clicked")

                        var dialogDate = pageStack.push(pickerDate, { date: new Date(nowDate) })
                        dialogDate.accepted.connect(function() {
                            console.log("You chose:", dialogDate.dateText)
                            // use date, as dateText return varies
                            var d = dialogDate.date
                            updateDateTime(Qt.formatDateTime(new Date(d), "yyyy-MM-dd"), nowTime)
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
                        var h = Qt.formatDateTime(new Date(nowDate + " " + nowTime), "hh")
                        var m = Qt.formatDateTime(new Date(nowDate + " " + nowTime), "mm")

                        console.log("modifyTimeButton clicked")
                        console.log("hour", h)
                        console.log("minute", m)

                        var dialogTime = pageStack.push(pickerTime, {hour: h, minute: m})
                        dialogTime.accepted.connect(function() {
                            console.log("You chose:", dialogTime.timeText)
                            var tt = dialogTime.timeText + ":00"
                            if (dialogTime.hour.length < 2) tt = "0" + tt
                            updateDateTime(nowDate, tt)
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
                color: Theme.primaryColor
                placeholderText: qsTr("Enter new value here")
                onTextChanged: addValuePage.canAccept = text.length > 0
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
                color: Theme.primaryColor
                placeholderText: qsTr("Enter annotation here")
                EnterKey.enabled: valueField.text.length > 0
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: addValuePage.accept()
                onFocusChanged: if (focus) flick.scrollToBottom()
            }
        }
    }
}
