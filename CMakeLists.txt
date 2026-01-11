cmake_minimum_required(VERSION 3.22)
project(ProjetM1)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML 3 REQUIRED COMPONENTS
        Graphics
        Window
        System
        Network
)

add_executable(ProjetM1 main.cpp
        Utils.hpp
        communication/Server.cpp
        game/Position.cpp
)

target_link_libraries(ProjetM1 PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Network
)