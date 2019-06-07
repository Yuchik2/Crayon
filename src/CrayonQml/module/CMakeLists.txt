cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(QmlModule)

set(SOURCES
    crqmlnodeextension.cpp
    crqmlmoduleplugin.cpp
    )

set(HEADERS
    crqmlnodeextension.h
    crqmlmoduleplugin.h
    )

add_library(CrayonQmlModule MODULE ${SOURCES} ${HEADERS} qmldir)
target_link_libraries(CrayonQmlModule PRIVATE CrayonQmlLib)

set(ModuleName "Crayon")
set(PluginName "crqmlmoduleplugin")

set_property(TARGET CrayonQmlModule PROPERTY OUTPUT_NAME ${PluginName})
set_property(TARGET CrayonQmlModule PROPERTY PREFIX "")

set_property(TARGET CrayonQmlModule PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR}/qml/${ModuleName})

configure_file(qmldir ${CRAYON_BIN_DIR}/qml/${ModuleName}/qmldir)





