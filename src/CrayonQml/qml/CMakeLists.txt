cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(CrayonQmlModules)

find_package(Qt5 REQUIRED LinguistTools)

include(../../../common.cmake)

set(MODULES
    Gauge.qml
    Slider.qml
    )

add_custom_target(${PROJECT_NAME} ALL
    SOURCES ${MODULES})

foreach(FILENAME ${MODULES})
    set(SRC "${CMAKE_CURRENT_LIST_DIR}/${FILENAME}")
    set(DST "${CRAYON_BIN_DIR}/${CRAYON_INSTALL_CRAYON_QML_MODULES_PREFIX}/${FILENAME}")

    add_custom_command(
        TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC} ${DST}
        )
endforeach(FILENAME)

install(
    FILES  ${MODULES}
    DESTINATION ${CRAYON_INSTALL_CRAYON_QML_MODULES_PREFIX})

qt5_create_translation(
    QM_FILES
    ${MODULES}
    ${PROJECT_NAME}_${TRANSLATE_SUFFIX}.ts)
set_property(DIRECTORY PROPERTY CLEAN_NO_CUSTOM TRUE)

add_custom_target(
    translateCrayonQmlModules
    DEPENDS ${QM_FILES})

install(
    FILES  ${QM_FILES}
    DESTINATION ${CRAYON_INSTALL_TRANSLATIONS_PREFIX}/${TRANSLATE_SUFFIX})


