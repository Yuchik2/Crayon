
set(CRAYON_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src CACHE PATH "Crayon source directory")
set(CRAYON_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include CACHE PATH "Crayon include directory")
set(CRAYON_BIN_DIR ${CMAKE_CURRENT_LIST_DIR}/bin/${CMAKE_BUILD_TYPE} CACHE PATH "Crayon binary directory")
set(CRAYON_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_BUILD_TYPE} CACHE PATH "Crayon library directory")

set(CRAYON_INSTALL_QT_PLUGINS_PREFIX "QtPlugins" CACHE PATH "Install Qt plugins prefix")
set(CRAYON_INSTALL_QML_MODULES_PREFIX "QmlModules" CACHE PATH "Install Qml modules prefix")
set(CRAYON_INSTALL_PLUGINS_PREFIX "CrayonPlugins" CACHE PATH "Install Crayon plugins prefix")
set(CRAYON_INSTALL_CRAYON_QML_MODULES_PREFIX "CrayonQmlModules" CACHE PATH "Install Crayon Qml modules prefix")
set(CRAYON_INSTALL_TRANSLATIONS_PREFIX "Translations" CACHE PATH "Install translations prefix")

set(QT_PLUGINS_DIR ${CMAKE_PREFIX_PATH}/plugins CACHE PATH "Qt plugins")
set(QT_BIN_DIR ${CMAKE_PREFIX_PATH}/bin CACHE PATH "Qt bin")
set(QT_QML_DIR ${CMAKE_PREFIX_PATH}/qml CACHE PATH "Qt qml")

set(QT_PLATFORMS_PLUGIN_DEBUG ${QT_PLUGINS_DIR}/platforms/qwindowsd.dll)
set(QT_PLATFORMS_PLUGIN_RELEASE ${QT_PLUGINS_DIR}/platforms/qwindows.dll)

set(QT_STYLES_PLUGIN_DEBUG ${QT_PLUGINS_DIR}/styles/qwindowsvistastyled.dll)
set(QT_STYLES_PLUGIN_RELEASE ${QT_PLUGINS_DIR}/styles/qwindowsvistastyle.dll)

set(QT_ICONENGINES_PLUGIN_DEBUG ${QT_PLUGINS_DIR}/iconengines/qsvgicond.dll)
set(QT_ICONENGINES_PLUGIN_RELEASE ${QT_PLUGINS_DIR}/iconengines/qsvgicon.dll)

set(TRANSLATE_SUFFIX ru_RU CACHE STRING "Translate suffix")

set(CMAKE_INCLUDE_PATH ${CRAYON_INCLUDE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CRAYON_BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CRAYON_LIB_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CRAYON_LIB_DIR})

set(CRAYON_BIN_PLUGINS_DIR ${CRAYON_BIN_DIR}/plugins CACHE PATH "Crayon plugins")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(QT_CONFIG_LIB_SUFFIX "d")
else()
    set(QT_CONFIG_LIB_SUFFIX "")
endif()
