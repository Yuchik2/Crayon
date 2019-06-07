cmake_minimum_required(VERSION 2.8)

project(CrayonStandartPlugin)

find_package(Qt5 REQUIRED Core Gui Widgets SerialPort)

include(../../common.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR}/CrayonPlugins)


set(SOURCES
    crstandartplugin.cpp
    crgraphicsscenenode.cpp
    crserialportnode.cpp
    )

set(HEADERS
    ${CRAYON_INCLUDE_DIR}/Plugins/CrayonStandartPlugin/crayonstandartplugin_global.h
    ${CRAYON_INCLUDE_DIR}/Plugins/CrayonStandartPlugin/crserialportnode.h
    crstandartplugin.h
    crgraphicsscenenode.h
    byteconverters.h
    )

set(CMAKE_AUTOMOC ON)

qt5_add_resources(QRC_WRAPPERS resources/resources.qrc OPTIONS -no-compress)

add_library(CrayonStandartPlugin SHARED ${SOURCES} ${HEADERS} ${QRC_WRAPPERS} resources/resources.qrc)
set_property(TARGET CrayonStandartPlugin PROPERTY OUTPUT_NAME CrayonStandartPlugin)

target_compile_definitions(CrayonStandartPlugin PRIVATE -DCRAYON_STANDART_PLUGIN_LIBRARY)
target_include_directories(CrayonStandartPlugin
    PUBLIC
    ${CRAYON_INCLUDE_DIR}/CrayonCore
    ${CRAYON_INCLUDE_DIR}/CrayonCore/Nodes
    ${CRAYON_INCLUDE_DIR}/CrayonCore/Utils
    ${CRAYON_INCLUDE_DIR}/Plugins/CrayonStandartPlugin
    )

target_link_libraries(CrayonStandartPlugin PRIVATE ${CRAYON_LIB_DIR}/libCrayonCore.dll.a)

set(COMPILE_OPTIONS -std=c++14)
target_compile_options(CrayonStandartPlugin PRIVATE ${COMPILE_OPTIONS})

qt5_use_modules(CrayonStandartPlugin
    Core
    Gui
    Widgets
    SerialPort
    )



