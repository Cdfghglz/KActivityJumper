cmake_minimum_required(VERSION 3.5)
project(ActivityJumper)

find_package(Qt5 CONFIG REQUIRED Core DBus)

set(SOURCE_FILES org.kde.ActivityJumper.xml)
qt5_generate_dbus_interface(AJump.hpp
    org.kde.ActivityJumper.xml
    OPTIONS -A
)
qt5_add_dbus_adaptor(SOURCE_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ActivityJumper.xml
    AJump.hpp
    ActivityJumper
)

qt5_wrap_cpp(ajump_moc AJump.hpp)
add_executable(ActivityJumper
    ${ajump_moc}
    app.cpp
    AJump.cpp
    ${SOURCE_FILES}
)
target_include_directories(ActivityJumper
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(ActivityJumper
    Qt5::DBus
)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ActivityJumper.xml DESTINATION "${DBUS_INTERFACES_INSTALL_DIR}")