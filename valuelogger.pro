#
# Project valuelogger, Value Logger
#

TARGET = harbour-valuelogger

CONFIG += sailfishapp

QT += sql

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

CONFIG(debug, debug|release) {
    DEFINES += DEBUG LOG_DBG
}

SOURCES += src/valuelogger.cpp \
        src/graph.cpp \
        src/logger.cpp

HEADERS += src/debuglog.h \
        src/graph.h \
        src/logger.h

OTHER_FILES += qml/valuelogger.qml \
    rpm/valuelogger.spec \
    harbour-valuelogger.desktop \
    qml/cover/*.qml \
    qml/pages/*.qml \
    qml/components/*.qml \
    qml/images/*.svg \
    qml/js/*.js \
    qml/icon-cover-plot.png \
    translations/harbour-valuelogger.ts

CONFIG += sailfishapp_i18n sailfishapp_i18n_unfinished
TRANSLATIONS += \
    translations/harbour-valuelogger_fi.ts \
    translations/harbour-valuelogger_sv.ts

# Icons
ICON_SIZES = 86 108 128 256
for(s, ICON_SIZES) {
    icon_target = icon$${s}
    icon_dir = icons/$${s}x$${s}
    $${icon_target}.files = $${icon_dir}/$${TARGET}.png
    $${icon_target}.path = /usr/share/icons/hicolor/$${s}x$${s}/apps
    INSTALLS += $${icon_target}
}
