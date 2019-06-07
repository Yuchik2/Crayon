cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(CrayonCore)

find_package(Qt5 REQUIRED Core Gui Widgets LinguistTools)

include(../../common.cmake)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    crcore.cpp
    crnode.cpp
    crproject.cpp
    crmonitor.cpp
    crlog.cpp
    crplugin.cpp
    crmodule.cpp
    crcommands.cpp

    crprojectmodel.cpp
    crprojectexplorerdialog.cpp

    crprojectmodulesmodel.cpp
    crprojectmodulesexplorerdialog.cpp

    crnodesettingsdialog.cpp
    crnodeeditorwidget.cpp
    crprojectsettingsdialog.cpp
    )

set(HEADERS
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crayoncore_global.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crcore.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crnode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crproject.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crmonitor.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crlog.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crplugininterface.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crplugin.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crmodule.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crcommands.h

    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crprojectmodel.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crprojectexplorerdialog.h

    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crprojectmodulesmodel.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crprojectmodulesexplorerdialog.h

    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crnodesettingsdialog.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crnodeeditorwidget.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crprojectsettingsdialog.h
    )

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${QRC_WRAPPERS})
set_property(TARGET ${PROJECT_NAME} PROPERTY PREFIX "")
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR})
#set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DCRAYON_CORE_LIBRARY)
target_compile_options(${PROJECT_NAME} PUBLIC -std=gnu++14)

#set_property(TARGET CrayonCore PROPERTY OUTPUT_NAME CrCore)

get_property(
    LIB_QT5_CORE_PATH
    TARGET Qt5::Core
    PROPERTY IMPORTED_LOCATION_${CONFIG})

install(
    FILES  ${LIB_QT5_CORE_PATH}
    DESTINATION ".")

get_property(
    LIB_QT5_GUI_PATH
    TARGET Qt5::Gui
    PROPERTY IMPORTED_LOCATION_${CONFIG})

install(
    FILES  ${LIB_QT5_GUI_PATH}
    DESTINATION ".")

get_property(
    LIB_QT5_WIDGETS_PATH
    TARGET Qt5::Widgets
    PROPERTY IMPORTED_LOCATION_${CONFIG})

install(
    FILES  ${LIB_QT5_WIDGETS_PATH}
    DESTINATION ".")

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME
    DESTINATION ".")


qt5_create_translation(
    QM_FILES ${HEADERS} ${SOURCES} ${PROJECT_NAME}_${TRANSLATE_SUFFIX}.ts)
set_property(DIRECTORY PROPERTY CLEAN_NO_CUSTOM TRUE)

add_custom_target(
    translateCrayonCore
    DEPENDS ${QM_FILES})

install(
    FILES  ${QM_FILES}
    DESTINATION ${CRAYON_INSTALL_TRANSLATIONS_PREFIX}/${TRANSLATE_SUFFIX})

