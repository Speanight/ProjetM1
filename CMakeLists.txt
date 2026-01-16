cmake_minimum_required(VERSION 3.22)
project(ProjetM1)

set(CMAKE_CXX_STANDARD 20)

# Matches how SFML and ImGui-SFML were built:
set(SFML_STATIC_LIBRARIES OFF)

find_package(SFML 3 REQUIRED COMPONENTS
        Graphics
        Window
        System
        Network
)

find_package(OpenGL REQUIRED)
find_package(ImGui-SFML REQUIRED)

add_executable(ProjetM1 main.cpp
        Utils.hpp
        communication/Server.cpp
        game/Position.cpp
        communication/Client.cpp
        communication/Client.hpp
)

target_link_libraries(ProjetM1 PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Network

        ImGui-SFML::ImGui-SFML
)