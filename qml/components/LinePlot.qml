/*
	Original Copyright (c) 2013 Jussi Sainio
 
	Modified to support multiple lines for valuelogger 2014 Kimmo Lindholm

    More or less completely rewritten in 2021 by Slava Monich

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

Item {
    property var dataListModel
    property var parInfoModel
    property string column: "value"
    property bool dragging

    property real min: 0.0
    property real max: 1.0

    property date xstart: new Date()
    property date xend: new Date()

    readonly property int thinLine: Math.max(2, Math.floor(Theme.paddingSmall/3))
    readonly property int fontSize: Theme.fontSizeTiny
    readonly property bool fontBold: true

    function distanceX(p1, p2){
        return Math.max(p1.x, p2.x) - Math.min(p1.x, p2.x)
    }

    function distanceY(p1, p2) {
        return Math.max(p1.y, p2.y) - Math.min(p1.y, p2.y)
    }

    function getMinMax(data) {
        if (data.length > 0) {
            var last = data.length - 1;
            var first = 0;

            var s = new Date(data[0]["timestamp"])

            if (s.getTime() < xstart.getTime())
                xstart = s

            s = new Date(data[data.length-1]["timestamp"])

            if (s.getTime() > xend.getTime())
                xend = s

            first = 0;
            last = data.length - 1;

            for (var i = first; i <= last; i++) {
                var l = data[i]

                if (l[column] > max) max = l[column]
                if (l[column] < min) min = l[column]
            }
        }
    }

    function updateVerticalScale() {
        var m = (((max-min))/canvas.height)*pinchZoom.deltaY

        max = max - m/2
        min = min + m/2

        var d = (((max-min))/canvas.height)*pinchMove.movementY

        max = max + d
        min = min + d

        valueMax.text = max.toFixed(2)
        valueMin.text = min.toFixed(2)

        var n = valueMiddle.count + 1
        for (var midIndex=0; midIndex<valueMiddle.count; midIndex++) {
            valueMiddle.itemAt(midIndex).text = (min+(((max-min)/n)*(midIndex+1))).toFixed(2)
        }
    }

    function updateHorizontalScale() {
        var mm = (((xstart.getTime() - xend.getTime()))/canvas.width)*pinchZoom.deltaX

        var t = new Date()
        t.setTime(xstart.getTime() - Math.floor(mm))
        xstart = t

        var u = new Date()
        u.setTime(xend.getTime() + Math.floor(mm))
        xend = u

        var dd = (((xstart.getTime() - xend.getTime()))/canvas.width)*pinchMove.movementX

        t = new Date()
        t.setTime(xstart.getTime() + Math.floor(dd))
        xstart = t

        u = new Date()
        u.setTime(xend.getTime() + Math.floor(dd))
        xend = u

        xStart.text = Qt.formatDateTime(xend, "dd.MM.yyyy hh:mm")
        xEnd.text = Qt.formatDateTime(xstart, "dd.MM.yyyy hh:mm")
    }

    function updateGraph() {
        // assign some timestamp which is in range as start/end default for further expanding
        xstart = new Date(dataListModel[0][0]["timestamp"])
        xend = new Date(dataListModel[0][0]["timestamp"])

        min = 99999999.9
        max = -99999999.9

        for (var n=0; n<dataListModel.length; n++)
            getMinMax(dataListModel[n])

        updateVerticalScale()
        updateHorizontalScale()
    }

    onWidthChanged: sizeChangedTimer.restart()
    onHeightChanged: sizeChangedTimer.restart()

    Timer {
        id: sizeChangedTimer
        interval: 0
        repeat: false
        onTriggered: updateGraph()
    }

    Text {
        id: xStart
        color: Theme.primaryColor
        font.pixelSize: fontSize
        font.bold: fontBold
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.top: parent.top
    }

    Text {
        id: xEnd
        color: Theme.primaryColor
        font.pixelSize: fontSize
        font.bold: fontBold
        anchors.left: parent.left
        anchors.top: parent.top
        horizontalAlignment: Text.AlignRight
    }

    Text {
        id: valueMax
        color: Theme.primaryColor
        font.pixelSize: fontSize
        font.bold: fontBold
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingSmall
        anchors.top: xEnd.bottom
    }

    Text {
        id: valueMin
        color: Theme.primaryColor
        font.pixelSize: fontSize
        font.bold: fontBold
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingSmall
        anchors.bottom: parent.bottom
    }

    Repeater {
        id: valueMiddle
        model:4

        Text {
            color: Theme.primaryColor
            font.pixelSize: fontSize
            font.bold: fontBold
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingSmall
            y: valueMin.y + (index+1)*(valueMax.y + valueMax.height - valueMin.y)/5
            z: 10
        }
    }

    ListView {
        id: legend
        x: parent.width/12
        y: x
        z: 11
        width: parent.width - 2 * x
        height: contentHeight
        model: parInfoModel

        delegate: ListItem {
            id: legendItem
            height: Math.floor(fontSize * 3 / 2)
            width: legend.width

            Rectangle {
                id: legendColor
                width: Theme.paddingLarge
                height: Theme.paddingSmall/2
                color: modelData.plotcolor
                anchors.verticalCenter: parent.verticalCenter
            }
            Label {
                text: modelData.name
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: legendColor.right
                    leftMargin: Theme.paddingSmall
                    right: parent.right
                }
                truncationMode: TruncationMode.Fade
                font {
                    pixelSize: fontSize
                    bold: fontBold
                }
            }
        }

        Behavior on opacity {
            FadeAnimation {}
        }

        onOpacityChanged: {
            if (opacity == 1.0)
                legendVisibility.start()
        }

        Timer {
            id: legendVisibility
            interval: 2000
            running: true
            onTriggered:  legend.opacity = 0.0
        }
    }

    ShaderEffectSource {
        id: grid
        width: parent.width
        anchors {
            top: valueMax.bottom
            bottom: valueMin.top
        }
        opacity: 0.3
        sourceItem: Item {
            width: grid.width
            height: grid.height
            Repeater {
                model: 7
                delegate: Rectangle {
                    x: Math.round(index * (parent.width - width)/6)
                    width: thinLine
                    height: parent.height
                    color: Theme.primaryColor
                }
            }
            Repeater {
                model: 6
                delegate: Rectangle {
                    y: Math.round(index * (parent.height - height)/5)
                    width: parent.width
                    height: thinLine
                    color: Theme.primaryColor
                }
            }
        }
    }

    Item {
        id: canvas
        width: parent.width
        anchors {
            top: valueMax.bottom
            bottom: valueMin.top
        }
        PinchArea {
            id: pinchZoom
            anchors.fill: canvas

            property real iX
            property real iY
            property real deltaX : 0
            property real deltaY : 0

            property point lv1
            property point lv2

            property bool scaleInX

            onPinchFinished: {
            }
            onPinchStarted: {
                iX = distanceX(pinch.point1, pinch.point2)
                iY = distanceY(pinch.point1, pinch.point2)

                scaleInX = (iX > iY)
            }
            onPinchUpdated: {
                if (pinch.point1 !== pinch.point2) {
                    lv1 = pinch.point1
                    lv2 = pinch.point2
                }
                if (scaleInX) {
                    var dX = distanceX(lv1, lv2) - iX
                    iX = distanceX(lv1, lv2)
                    deltaX += dX
                } else {
                    var dY = distanceY(lv1, lv2) - iY
                    iY = distanceY(lv1, lv2)
                    deltaY += dY
                }
                updateGraph()
            }

            MouseArea {
                property real iX
                property real iY
                property real movementX : 0
                property real movementY : 0

                id: pinchMove
                anchors.fill: parent

                onClicked: legend.opacity = 1.0

                onPressed: {
                    dragging = true
                    iX = mouseX
                    iY = mouseY
                }
                onDoubleClicked: {
                    movementX = 0
                    movementY = 0
                    pinchZoom.deltaX = 0
                    pinchZoom.deltaY = 0

                    updateGraph()
                }
                onPositionChanged: {
                    var dX = mouseX - iX
                    iX = mouseX
                    movementX += dX
                    var dY = mouseY - iY
                    iY = mouseY
                    movementY += dY

                    updateGraph()
                }
                onReleased: dragging = false
                onCanceled: dragging = false
            }
        }

        Repeater {
            model: dataListModel
            delegate: Graph {
                anchors.fill: parent
                minValue: min
                maxValue: max
                minTime: xstart
                maxTime: xend
                data: modelData
                lineWidth: Math.max(Math.min(Math.round(Theme.paddingSmall/2), width/modelData.length), 2)
                color: parInfoModel[index].plotcolor
            }
        }
    }
}
