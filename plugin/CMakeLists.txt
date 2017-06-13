


find_package(Qt5 CONFIG REQUIRED Core DBus)
find_package(KF5WindowSystem)

set(plasmoidplugin_SRCS
    plasmoidplugin.cpp
    )

add_library(plasmoidplugin SHARED ${plasmoidplugin_SRCS})

target_link_libraries(plasmoidplugin
        Qt5::Quick
        KF5::Plasma
        KF5::WindowSystem
        )

target_link_libraries(plasmoidplugin
    Qt5::DBus
)
install(TARGETS plasmoidplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/private/activityjumper)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/private/activityjumper)
