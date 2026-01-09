cmake_minimum_required(VERSION 3.22)
project(ProjetM1)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)


add_executable(ProjetM1 main.cpp)
target_link_libraries(ProjetM1
        sfml-graphics
        sfml-window
        sfml-system
        sfml-network
)
