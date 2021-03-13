import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.valuelogger 1.0

import "js/debug.js" as Debug
import "pages"
import "cover"

ApplicationWindow {
    id: valuelogger

    property var plotColors:[ "#ffffff", "#ff0080", "#ff8000", "#ffff00", "#00ff00",
                              "#00ff80", "#00ffff", "#0000ff", "#8000ff", "#ff00ff" ]

    initialPage: Component {
        MainPage {
            allowedOrientations: valuelogger.allowedOrientations
            onPlotSelected: valuelogger.plotSelected(PageStackAction.Animated)
        }
    }

    cover: Component {
        CoverPage {
            onAddValue: {
                valuelogger.addValue()
                valuelogger.activate()
            }
            onPlotSelected: {
                popAll()
                if (valuelogger.plotSelected(PageStackAction.Immediate)) {
                    valuelogger.activate()
                }
            }
        }
    }

    function popAll() {
        /* Remove all except bottom page */
        Debug.log("stack depth", pageStack.depth)
        while (pageStack.depth > 1) {
            pageStack.pop(null, PageStackAction.Immediate)
            Debug.log("stack depth", pageStack.depth)
        }
    }

    function addValue() {
        if (Logger.defaultParameterIndex >= 0) {
            var par = Logger.get(Logger.defaultParameterIndex)
            Debug.log("Adding", par.name,"value")

            popAll()

            var dialog
            dialog = pageStack.push(Qt.resolvedUrl("pages/AddValue.qml"), {
                "allowedOrientations": allowedOrientations,
                "parameterName": par.name,
                "parameterDescription": par.description
            }, PageStackAction.Immediate)

            var paired
            if (par.pairedtable) {
                Debug.log("this is a paired parameter")
                for (var i = 0; i < Logger.count; i++) {
                    var par2 = Logger.get(i)
                    if (par2.datatable === par.pairedtable) {
                        paired = par2
                        Debug.log("found", par2.name, par2.description)
                        break
                    }
                }
            }

            if (paired) {
                var pairdialog = pageStack.pushAttached(Qt.resolvedUrl("pages/AddValue.qml"), {
                    "allowedOrientations": allowedOrientations,
                    "nowDate": dialog.nowDate,
                    "nowTime": dialog.nowTime,
                    "parameterName": paired.name,
                    "parameterDescription": paired.description,
                    "paired": true
                })

                pairdialog.accepted.connect(function() {
                    var time = dialog.nowDate + " " + dialog.nowTime
                    var pairedtime = pairdialog.nowDate + " " + pairdialog.nowTime

                    Debug.log("paired dialog accepted")
                    Debug.log(" value", dialog.value)
                    Debug.log(" annotation", dialog.annotation)
                    Debug.log(" time", time)
                    Debug.log("  paired value", pairdialog.value)
                    Debug.log("  paired annotation", pairdialog.annotation)
                    Debug.log("  paired time", pairedtime)

                    Logger.addData(par.datatable, dialog.value, dialog.annotation, time)
                    Logger.addData(par.pairedtable, pairdialog.value, pairdialog.annotation, pairedtime)
                    valuelogger.deactivate()
                })
            } else {
                dialog.accepted.connect(function() {
                    var time = dialog.nowDate + " " + dialog.nowTime

                    Debug.log("dialog accepted")
                    Debug.log(" value", dialog.value)
                    Debug.log(" annotation", dialog.annotation)
                    Debug.log(" time", time)

                    Logger.addData(par.datatable, dialog.value, dialog.annotation, time)
                    valuelogger.deactivate()
                })
            }

            dialog.rejected.connect(function() {
                Debug.log("Dialog rejected")
                valuelogger.deactivate()
            })
        }
    }

    function plotSelected(operationType) {
        if (Logger.count > 0) {
            var parInfo = []

            Debug.log(Logger.count, "item(s) in list.")
            for (var i = 0; i < Logger.count; i++) {
                var par = Logger.get(i)
                if (par.visualize) {
                    Debug.log("showing", par.name)
                    parInfo.push({
                        "name": par.name,
                        "plotcolor": par.plotcolor,
                        "datatable": par.datatable
                    })
                }
            }

            pageStack.push(Qt.resolvedUrl("pages/DrawData.qml"), {
                "allowedOrientations": allowedOrientations,
                "parInfo": parInfo
            }, operationType)

            return true
        } else {
            return false
        }
    }
}
