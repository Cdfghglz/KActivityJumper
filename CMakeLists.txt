cmake_minimum_required(VERSION 3.5)

# --------- SERVICE
project(ActivityJumper)

find_package(Qt5 CONFIG REQUIRED Core DBus)

set(SOURCE_FILES
        org.kde.ActivityJumper.xml

        src/error.hpp
        src/activityjumper.hpp
        src/common.h)

set(SOURCE_FILES ${SOURCE_FILES}
        src/error.cpp
        src/app.cpp
        src/activityjumper.cpp
        )

qt5_generate_dbus_interface(src/activityjumper.hpp
    org.kde.ActivityJumper.xml
    OPTIONS -A
)
qt5_add_dbus_adaptor(SOURCE_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ActivityJumper.xml
        src/activityjumper.hpp
    ActivityJumper
)

qt5_wrap_cpp(ajump_moc src/activityjumper.hpp)
add_executable(ActivityJumper
    ${ajump_moc}
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
set(CMAKE_MODULE_PATH
        ${ECM_MODULE_PATH}
        ${ECM_KDE_MODULE_DIR}
        ${CMAKE_MODULE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        )

include(KDEInstallDirs)
include(DBusMacros)

# install
configure_file(config/jumperDestinationConfig.config ~/.ActivityJumper/jumperDestinationConfig.config COPYONLY)
install (TARGETS ActivityJumper DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
dbus_add_activation_service(config/org.kde.ActivityJumper.service.in)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ActivityJumper.xml DESTINATION "${DBUS_INTERFACES_INSTALL_DIR}")

# --------- PLASMOID
set(QT_MIN_VERSION "5.4.0")
set(KF5_MIN_VERSION "5.0.0")

find_package(KF5Plasma)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Quick)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Plasma)

include(KDECMakeSettings)
include(KDECompilerSettings)

add_subdirectory(plugin)

plasma_install_package(package org.kde.plasma.activityjumper)

install(DIRECTORY package/icons/breeze_ package/icons/breeze DESTINATION ${ICON_INSTALL_DIR})
