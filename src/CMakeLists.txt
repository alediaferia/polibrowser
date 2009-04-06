include_directories(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
)

find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})

set( polibrowser_SRCS
     mainwindow.cpp
     main.cpp
 )
# kde4_add_ui_files(gluk_SRCS main.ui packagedock.ui installdock.ui instresumedock.ui)

kde4_add_executable(polibrowser ${polibrowser_SRCS})

target_link_libraries(polibrowser ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QT_QTWEBKIT_LIBRARY})

install(TARGETS polibrowser DESTINATION ${BIN_INSTALL_DIR})
install(FILES polibrowserui.rc
        DESTINATION ${DATA_INSTALL_DIR}/polibrowser)
