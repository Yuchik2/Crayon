cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(CrayonModbus)

find_package(Qt5 REQUIRED Core Gui Widgets SerialPort LinguistTools)

include(../../common.cmake)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    crmodbusrequest.cpp
    crmodbusabstractclient.cpp
    crmodbusrtuclient.cpp
    crmodbustcpclient.cpp
    crmodbusclientnode.cpp
    )

set(HEADERS
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crayonmodbus_global.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crmodbusrequest.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crmodbusabstractclient.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crmodbusrtuclient.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crmodbustcpclient.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crmodbusclientnode.h
    )



qt5_add_resources(QRC_WRAPPERS resources/resources.qrc OPTIONS -no-compress)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${QRC_WRAPPERS} resources/resources.qrc)
set_property(TARGET ${PROJECT_NAME} PROPERTY PREFIX "")
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR})
#set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME} )
target_link_libraries(${PROJECT_NAME} PUBLIC CrayonStandart)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DCRAYON_MODBUS_LIBRARY)
target_compile_options(${PROJECT_NAME} PUBLIC -std=c++14)

install(
TARGETS ${PROJECT_NAME}
RUNTIME
DESTINATION ".")



add_library(${PROJECT_NAME}Plugin MODULE plugin/crmodbusplugin.cpp plugin/crmodbusplugin.h )
set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY PREFIX "")
set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR}/${CRAYON_INSTALL_PLUGINS_PREFIX})
#set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY OUTPUT_NAME ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}Plugin PRIVATE ${PROJECT_NAME})

install(
TARGETS ${PROJECT_NAME}Plugin
DESTINATION ${CRAYON_INSTALL_PLUGINS_PREFIX})



qt5_create_translation(
QM_FILES
${SOURCES}
${HEADERS}
plugin/crmodbusplugin.cpp
plugin/crmodbusplugin.h
${PROJECT_NAME}_${TRANSLATE_SUFFIX}.ts)
set_property(DIRECTORY PROPERTY CLEAN_NO_CUSTOM TRUE)

add_custom_target(
translateCrayonModbus
DEPENDS ${QM_FILES})

install(
FILES  ${QM_FILES}
DESTINATION ${CRAYON_INSTALL_TRANSLATIONS_PREFIX}/${TRANSLATE_SUFFIX})
