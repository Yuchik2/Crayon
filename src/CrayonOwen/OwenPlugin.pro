include( ../../common.pri )
include( ../../lib.pri )

QT += core widgets serialport

TARGET = OwenPlugin$${LIB_SUFFIX}
TEMPLATE = lib
CONFIG += plugin

DLLDESTDIR += $${BIN_PATH}/CrayonPlugins

SOURCES += \   
    owenplugin.cpp

HEADERS += \
    owenplugin.h

LIBS += -lCrayonCore$${LIB_SUFFIX} -lCrayonStandartPlugin$${LIB_SUFFIX} -lCrayonModbusPlugin$${LIB_SUFFIX}

win32 {
    QMAKE_TARGET_PRODUCT = Crayon
    QMAKE_TARGET_DESCRIPTION = Crayon modbus library
}

DISTFILES += \  
    OwenPlugin.json

TRANSLATIONS = CrayonModbusPlugin_ru.ts

RESOURCES += \
    resources/Resources.qrc
