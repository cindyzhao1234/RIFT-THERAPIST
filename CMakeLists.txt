cmake_minimum_required(VERSION 3.16)

project(RiftTherapist)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find the installed libraries on my computer. Required = if CMAKE cannot find, stop and show error
find_package(cpr REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(rift_therapist
    src/main.cpp
)

target_link_libraries(rift_therapist
    PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
)