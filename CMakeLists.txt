cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(UTM_APP)

find_package(Threads REQUIRED)

if(NOT MSVC)
    add_definitions("-Wall -Wextra")
else()
    add_definitions("-WX -W2")
endif()

find_package(MAVSDK REQUIRED)


add_executable(UTM_APP 
UTM_APP.cpp GCS_UTM.cpp UAV_MAV.cpp 
)

include_directories(include)

target_link_libraries(UTM_APP
    MAVSDK::mavsdk        
    ${CMAKE_THREAD_LIBS_INIT}
)
