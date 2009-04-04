project(polibrowser)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})

add_subdirectory(src)
