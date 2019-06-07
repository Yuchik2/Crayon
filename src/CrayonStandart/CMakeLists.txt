cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(CrayonStandart)

find_package(Qt5 REQUIRED Core Gui Widgets SerialPort LinguistTools)

include(../../common.cmake)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    crnumericnode.cpp
    crbooleannode.cpp
    crtextnode.cpp
    crbytearraynode.cpp

    crpointnode.cpp
    crsizenode.cpp
    crrectnode.cpp

    crcolornode.cpp
    crfontnode.cpp
    crfilenode.cpp
    crcomboboxnode.cpp

    crserialportnode.cpp

    crgraphicsscenenode.cpp
    crgraphicsscene.cpp
    crgraphicsscenenodeeditor.cpp
    crgraphicsscenenodeitem.cpp
    )

set(HEADERS
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crayonstandart_global.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crnumericnode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crbooleannode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crtextnode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crbytearraynode.h

    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crpointnode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crsizenode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crrectnode.h

    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crcolornode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crfontnode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crfilenode.h
    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crcomboboxnode.h

    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crserialportnode.h

    ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME}/crgraphicsscenenode.h
    crgraphicsscene.h
    crgraphicsscenenodeeditor.h
    crgraphicsscenenodeitem.h
    )

qt5_add_resources(QRC_WRAPPERS resources/resources.qrc OPTIONS -no-compress)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${QRC_WRAPPERS} resources/resources.qrc)
set_property(TARGET ${PROJECT_NAME} PROPERTY PREFIX "")
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR})
#set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CRAYON_INCLUDE_DIR}/${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets Qt5::SerialPort CrayonCore)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DCRAYON_STANDART_LIBRARY)
target_compile_options(${PROJECT_NAME} PUBLIC -std=gnu++14)


install(
    FILES  ${QT_BIN_DIR}/Qt5SerialPort${QT_CONFIG_LIB_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}
    DESTINATION ".")


install(
    TARGETS ${PROJECT_NAME}
    RUNTIME
    DESTINATION ".")


message("${CMAKE_BUILD_TYPE}")

add_library(${PROJECT_NAME}Plugin MODULE plugin/byteconverters.h plugin/crstandartplugin.cpp plugin/crstandartplugin.h plugin/CrayonStandartPlugin.json )
set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY PREFIX "")
set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR}/${CRAYON_INSTALL_PLUGINS_PREFIX})
#set_property(TARGET ${PROJECT_NAME}Plugin PROPERTY OUTPUT_NAME ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}Plugin PRIVATE ${PROJECT_NAME})

install(
    TARGETS ${PROJECT_NAME}Plugin
    DESTINATION ${CRAYON_INSTALL_PLUGINS_PREFIX})



qt5_create_translation(
    QM_FILES ${SOURCES} ${HEADERS} plugin/byteconverters.h plugin/crstandartplugin.cpp plugin/crstandartplugin.h ${PROJECT_NAME}_${TRANSLATE_SUFFIX}.ts)
set_property(DIRECTORY PROPERTY CLEAN_NO_CUSTOM TRUE)

add_custom_target(
    translateCrayonStandart
    DEPENDS ${QM_FILES})

install(
    FILES  ${QM_FILES}
    DESTINATION ${CRAYON_INSTALL_TRANSLATIONS_PREFIX}/${TRANSLATE_SUFFIX})
