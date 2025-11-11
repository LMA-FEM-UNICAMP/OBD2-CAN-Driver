
cmake_minimum_required(VERSION 3.0)
project(obd2_can_driver)

find_package(Threads REQUIRED)

include_directories(include)

add_executable(
    obd2_can_driver_daemon 
    src/obd2_can_driver.cpp
    src/obd2_can_driver_daemon.cpp
    )

add_executable(
    obd2_can_driver_exe 
    src/obd2_can_driver.cpp
    src/obd2_can_driver_exe.cpp
    )

add_executable(
    obd2_can_driver_status 
    src/obd2_can_driver_status.cpp
    )

# Install executables in /opt folder

install(TARGETS obd2_can_driver_daemon DESTINATION /opt/obd2_can_driver/bin)
install(TARGETS obd2_can_driver_exe DESTINATION /opt/obd2_can_driver/bin)
install(TARGETS obd2_can_driver_status DESTINATION /opt/obd2_can_driver/bin)
install(CODE "execute_process(COMMAND ln -sf /opt/obd2_can_driver/bin/obd2_can_driver_status /usr/local/bin/obd2_can_driver_status)")