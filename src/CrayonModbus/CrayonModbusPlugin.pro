include( ../../common.pri )
include( ../../lib.pri )

QT += core widgets serialport

TARGET = CrayonModbusPlugin$${LIB_SUFFIX}
TEMPLATE = lib
CONFIG += plugin

DLLDESTDIR += $${BIN_PATH}/CrayonPlugins
DEFINES += CRAYON_MODBUS_PLUGIN_LIBRARY

SOURCES += \  
    crmodbusrequest.cpp \
    crnodemodbusrequest.cpp \
    crmodbusabstractclient.cpp \
    crmodbusrtuclient.cpp \
    crmodbusclientnode.cpp \
    crmodbusplugin.cpp


HEADERS += \
    $${INC_PATH}/CrayonModbusPlugin/crayonmodbusplugin_global.h \
    $${INC_PATH}/CrayonModbusPlugin/crmodbusrequest.h \
    $${INC_PATH}/CrayonModbusPlugin/crmodbusabstractclient.h \
    $${INC_PATH}/CrayonModbusPlugin/crmodbusrtuclient.h \
    $${INC_PATH}/CrayonModbusPlugin/crmodbusclientnode.h \
    crmodbusplugin.h



LIBS += -lCrayonCore$${LIB_SUFFIX} -lCrayonStandartPlugin$${LIB_SUFFIX}

win32 {
    QMAKE_TARGET_PRODUCT = Crayon
    QMAKE_TARGET_DESCRIPTION = Crayon modbus library
}

DISTFILES += \ 
    CrayonModbusPlugin.json

TRANSLATIONS = CrayonModbusPlugin_ru.ts

RESOURCES += \
    resources/Resources.qrc
