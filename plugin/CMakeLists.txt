cmake_minimum_required(VERSION 3.5)

find_package(Qt5 CONFIG REQUIRED Core DBus)
find_package(KF5WindowSystem)
find_package(KF5Activities)

set(plasmoidplugin_SRCS
    plasmoidplugin.cpp
    ../src/common.h
        )

add_library(plasmoidplugin SHARED ${plasmoidplugin_SRCS})

target_link_libraries(plasmoidplugin
        Qt5::Quick
        KF5::Plasma
        KF5::WindowSystem
        KF5::Activities
        )

target_link_libraries(plasmoidplugin
    Qt5::DBus
)
install(TARGETS plasmoidplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/private/activityjumper)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/private/activityjumper)
