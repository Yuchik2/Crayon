cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(CrayonOwen)

find_package(Qt5 REQUIRED Core Gui Widgets SerialPort LinguistTools)

include(../../common.cmake)

set(CMAKE_AUTOMOC ON)

qt5_add_resources(QRC_WRAPPERS resources/resources.qrc OPTIONS -no-compress)

add_library(${PROJECT_NAME}Plugin MODULE owenplugin.cpp owenplugin.h ${QRC_WRAPPERS} resources/resources.qrc)
set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY PREFIX "")
set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR}/${CRAYON_INSTALL_PLUGINS_PREFIX})
#set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY OUTPUT_NAME ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}Plugin PRIVATE CrayonModbus)

install(
TARGETS ${PROJECT_NAME}Plugin
DESTINATION ${CRAYON_INSTALL_PLUGINS_PREFIX})

qt5_create_translation(
QM_FILES owenplugin.cpp owenplugin.h ${PROJECT_NAME}_${TRANSLATE_SUFFIX}.ts)
set_property(DIRECTORY PROPERTY CLEAN_NO_CUSTOM TRUE)

add_custom_target(
translateCrayonOwen
DEPENDS ${QM_FILES})

install(
FILES  ${QM_FILES}
DESTINATION ${CRAYON_INSTALL_TRANSLATIONS_PREFIX}/${TRANSLATE_SUFFIX})
