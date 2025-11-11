
cmake_minimum_required(VERSION 3.0)
project(obd2_can_driver)

find_package(Threads REQUIRED)

include_directories(include)

add_executable(
    obd2_can_driver_deamon 
    src/obd2_can_driver.cpp
    src/obd2_can_driver_deamon.cpp
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