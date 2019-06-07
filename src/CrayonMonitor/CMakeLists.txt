cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(CrayonMonitor)

find_package(Qt5 REQUIRED Core Gui Widgets LinguistTools)

include(../../common.cmake)

set(CMAKE_AUTOMOC ON)

add_executable(${PROJECT_NAME} crmonitormainwindow.cpp crmonitormainwindow.h main.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY PREFIX "")
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR})
#set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets CrayonCore)
target_compile_options(${PROJECT_NAME} PUBLIC -std=gnu++14)



install(
TARGETS ${PROJECT_NAME}
RUNTIME
DESTINATION ".")

include(DeployQt4)
install_qt4_executable(
    "${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
    "styles;platforms"
    ""
    ${QT_BIN_DIR}
    ${CRAYON_INSTALL_QT_PLUGINS_PREFIX}
    )



qt5_create_translation(
QM_FILES crmonitormainwindow.cpp crmonitormainwindow.h main.cpp ${PROJECT_NAME}_${TRANSLATE_SUFFIX}.ts)
set_property(DIRECTORY PROPERTY CLEAN_NO_CUSTOM TRUE)

add_custom_target(
translateCrayonMonitor
DEPENDS ${QM_FILES})

install(
FILES  ${QM_FILES}
DESTINATION ${CRAYON_INSTALL_TRANSLATIONS_PREFIX}/${TRANSLATE_SUFFIX})
