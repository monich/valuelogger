#
# Value Logger, rewritten
#

TARGET = harbour-valuelogger2

CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp mlite5
QT += sql

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi

isEmpty(SPECVERSION) {
    SPECVERSION=1.0.21
}

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

CONFIG(debug, debug|release) {
    DEFINES += DEBUG LOG_DBG
}

SOURCES += src/colormodel.cpp \
        src/datatablemodel.cpp \
        src/dashline.cpp \
        src/database.cpp \
        src/datamodel.cpp \
        src/graph.cpp \
        src/hdashline.cpp \
        src/logger.cpp \
        src/pairmodel.cpp \
        src/settings.cpp \
        src/timegridmodel.cpp \
        src/valuegridmodel.cpp \
        src/valuelogger.cpp \
        src/vdashline.cpp

HEADERS += src/colormodel.h \
        src/datatablemodel.h \
        src/dashline.h \
        src/database.h \
        src/datamodel.h \
        src/debuglog.h \
        src/graph.h \
        src/hdashline.h \
        src/logger.h \
        src/pairmodel.h \
        src/settings.h \
        src/timegridmodel.h \
        src/valuegridmodel.h \
        src/vdashline.h

OTHER_FILES += qml/valuelogger.qml \
    rpm/valuelogger.spec \
    harbour-valuelogger2.desktop \
    qml/cover/*.qml \
    qml/pages/*.qml \
    qml/components/*.qml \
    qml/images/*.svg \
    qml/js/*.js \
    LICENSE \
    README.md

# Icons
ICON_SIZES = 86 108 128 172 256
for(s, ICON_SIZES) {
    icon_target = icon$${s}
    icon_dir = icons/$${s}x$${s}
    $${icon_target}.files = $${icon_dir}/$${TARGET}.png
    $${icon_target}.path = /usr/share/icons/hicolor/$${s}x$${s}/apps
    INSTALLS += $${icon_target}
}

# Translations
TRANSLATION_SOURCES = \
  $${_PRO_FILE_PWD_}/qml \
  $${_PRO_FILE_PWD_}/src

TARGET_DATA_DIR = /usr/share/$${TARGET}
TRANSLATIONS_PATH = $${TARGET_DATA_DIR}/translations
#TRANSLATION_IDBASED=-idbased

defineTest(addTrFile) {
    rel = translations/$${1}
    OTHER_FILES += $${rel}.ts
    export(OTHER_FILES)

    in = $${_PRO_FILE_PWD_}/$$rel
    out = $${OUT_PWD}/$$rel

    s = $$replace(1,-,_)
    lupdate_target = lupdate_$$s
    qm_target = qm_$$s

    $${lupdate_target}.commands = lupdate -noobsolete -locations none $${TRANSLATION_SOURCES} -ts \"$${in}.ts\" && \
        mkdir -p \"$${OUT_PWD}/translations\" &&  [ \"$${in}.ts\" != \"$${out}.ts\" ] && \
        cp -af \"$${in}.ts\" \"$${out}.ts\" || :

    $${qm_target}.path = $$TRANSLATIONS_PATH
    $${qm_target}.depends = $${lupdate_target}
    $${qm_target}.commands = lrelease $$TRANSLATION_IDBASED \"$${out}.ts\" && \
        $(INSTALL_FILE) \"$${out}.qm\" $(INSTALL_ROOT)$${TRANSLATIONS_PATH}/

    QMAKE_EXTRA_TARGETS += $${lupdate_target} $${qm_target}
    INSTALLS += $${qm_target}

    export($${lupdate_target}.commands)
    export($${qm_target}.path)
    export($${qm_target}.depends)
    export($${qm_target}.commands)
    export(QMAKE_EXTRA_TARGETS)
    export(INSTALLS)
}

LANGUAGES = fi pl ru sv

addTrFile($${TARGET})
for(l, LANGUAGES) {
    addTrFile($${TARGET}_$$l)
}
