cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(CrayonQml)

find_package(Qt5 REQUIRED Core Gui Widgets Qml Quick QuickWidgets LinguistTools)

include(../../common.cmake)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    crqmlnode.cpp
    crqmlcomponentnode.cpp
    crqmlmodule.cpp

    crqmlmonitornode.cpp
    crqmlmonitoritem.cpp
    crqmlmonitorwindow.cpp
    )

set(HEADERS
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crayonqml_global.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crqmlnode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crqmlcomponentnode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crqmlmodule.h

    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crqmlmonitornode.h
    crqmlmonitoritem.h
    crqmlmonitorwindow.h
    )

qt5_add_resources(QRC_WRAPPERS resources/resources.qrc OPTIONS -no-compress)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${QRC_WRAPPERS} resources/resources.qrc)
set_property(TARGET ${PROJECT_NAME} PROPERTY PREFIX "")
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR})
#set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME} )
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Qml Qt5::Quick Qt5::QuickWidgets CrayonStandart)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DCRAYON_QML_LIBRARY)
target_compile_options(${PROJECT_NAME} PUBLIC -std=c++14)

set(MODULES
    Qt5Network
    Qt5Qml
    Qt5Quick
    Qt5QuickWidgets
    Qt5QuickControls2
    Qt5QuickTemplates2
    )

foreach(MODULE ${MODULES})
    install(
        FILES  ${QT_BIN_DIR}/${MODULE}${QT_CONFIG_LIB_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}
        DESTINATION ".")
endforeach(MODULE)


install(
    TARGETS ${PROJECT_NAME}
    RUNTIME
    DESTINATION ".")

install(DIRECTORY ${QT_QML_DIR}/Qt DESTINATION ${CRAYON_INSTALL_QML_MODULES_PREFIX})
install(DIRECTORY ${QT_QML_DIR}/QtQml DESTINATION ${CRAYON_INSTALL_QML_MODULES_PREFIX})
install(DIRECTORY ${QT_QML_DIR}/QtQuick DESTINATION ${CRAYON_INSTALL_QML_MODULES_PREFIX})
install(DIRECTORY ${QT_QML_DIR}/QtQuick.2 DESTINATION ${CRAYON_INSTALL_QML_MODULES_PREFIX})
install(DIRECTORY ${QT_QML_DIR}/QtGraphicalEffects DESTINATION ${CRAYON_INSTALL_QML_MODULES_PREFIX})

add_library(${PROJECT_NAME}Plugin MODULE
    plugin/crqmlplugin.cpp
    plugin/crqmlplugin.h)
set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY PREFIX "")
set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR}/${CRAYON_INSTALL_PLUGINS_PREFIX})
#set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY OUTPUT_NAME ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}Plugin PRIVATE ${PROJECT_NAME})

install(
    TARGETS ${PROJECT_NAME}Plugin
    DESTINATION ${CRAYON_INSTALL_PLUGINS_PREFIX})



set(MODULE_NAME "Crayon")

add_library(${PROJECT_NAME}Module MODULE
    module/crqmlnodeextension.cpp
    module/crqmlnodeextension.h
    module/crqmlmoduleplugin.h
    module/crqmlmoduleplugin.cpp
    module/qmldir)
set_property(TARGET ${PROJECT_NAME}Module PROPERTY PREFIX "")
set_property(TARGET ${PROJECT_NAME}Module PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR}/${CRAYON_INSTALL_QML_MODULES_PREFIX}/${MODULE_NAME})

target_link_libraries(${PROJECT_NAME}Module PRIVATE ${PROJECT_NAME})

configure_file(module/qmldir ${CRAYON_BIN_DIR}/${CRAYON_INSTALL_QML_MODULES_PREFIX}/${MODULE_NAME}/qmldir)

install(
    FILES  ${CRAYON_BIN_DIR}/${CRAYON_INSTALL_QML_MODULES_PREFIX}/${MODULE_NAME}/qmldir
    DESTINATION ${CRAYON_INSTALL_QML_MODULES_PREFIX}/${MODULE_NAME})

install(
    TARGETS ${PROJECT_NAME}Module
    DESTINATION ${CRAYON_INSTALL_QML_MODULES_PREFIX}/${MODULE_NAME})



qt5_create_translation(
    QM_FILES
    ${SOURCES}
    ${HEADERS}
    plugin/crqmlplugin.cpp
    module/crqmlnodeextension.cpp
    module/crqmlmoduleplugin.cpp
    ${PROJECT_NAME}_${TRANSLATE_SUFFIX}.ts)
set_property(DIRECTORY PROPERTY CLEAN_NO_CUSTOM TRUE)

add_custom_target(
    translateCrayonQml
    DEPENDS ${QM_FILES})

install(
    FILES  ${QM_FILES}
    DESTINATION ${CRAYON_INSTALL_TRANSLATIONS_PREFIX}/${TRANSLATE_SUFFIX})

add_subdirectory(qml)


