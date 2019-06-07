include( ../../common.pri )
include( ../../app.pri )

QT += core gui widgets

TARGET = CrayonMonitor
TEMPLATE = app

SOURCES += \
    main.cpp \
    crmonitormainwindow.cpp


HEADERS += \
    crmonitormainwindow.h


LIBS += -lCrayonCore$${LIB_SUFFIX}\

