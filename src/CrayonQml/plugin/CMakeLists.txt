cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(QmlPlugin)

add_library(CrayonQmlPlugin MODULE crqmlplugin.cpp crqmlplugin.h)
target_link_libraries(CrayonQmlPlugin PRIVATE CrayonQmlLib)
set_property(TARGET CrayonQmlPlugin PROPERTY PREFIX "")
set_property(TARGET CrayonQmlPlugin PROPERTY OUTPUT_NAME CrQmlPlugin)
set_property(TARGET CrayonQmlPlugin PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR}/CrayonPlugins)


#file(COPY ${QM_FILES} DESTINATION ${CRAYON_BIN_DIR}/Translate/${TRANSLATE_SUFFIX})
