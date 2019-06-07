cmake_minimum_required(VERSION 2.8)

project(Crayon)

add_subdirectory(src/CrayonCore)
add_subdirectory(src/CrayonEditor)
add_subdirectory(src/CrayonMonitor)
add_subdirectory(src/CrayonStandart)
add_subdirectory(src/CrayonModbus)
add_subdirectory(src/CrayonQml)
add_subdirectory(src/CrayonOwen)


add_custom_target(
translateAll
DEPENDS
translateCrayonCore
translateCrayonEditor
translateCrayonMonitor
translateCrayonStandart
translateCrayonModbus
translateCrayonQml
translateCrayonQmlModules
translateCrayonOwen)





