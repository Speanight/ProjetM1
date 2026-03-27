cmake_minimum_required(VERSION 3.22)
project(ProjetM1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_STATIC_LIBRARIES OFF)

# =========================
# OpenGL
# =========================
find_package(OpenGL REQUIRED)

# =========================
# SFML 3
# =========================
find_package(SFML 3 REQUIRED COMPONENTS
        Graphics
        Window
        System
        Network
)

# =========================
# ImGui (local)
# =========================
find_package(ImGui-SFML REQUIRED)

# =========================
# X11 (for Multi-threading/keyboard)
# =========================
find_package(X11 REQUIRED)

# =========================
# Executable
# =========================
add_executable(ProjetM1
        main.cpp
        Utils.hpp
        communication/Server.cpp
        game/Position.cpp
        game/Weapon.cpp
        communication/Client.cpp
        communication/Client.hpp
        ui/ServerUI.cpp
        ui/MainWindow.cpp
        ui/ClientUI.cpp
        communication/State.cpp
        communication/Input.cpp
        game/gameUtils.cpp
        communication/Buffer.cpp
        communication/Buffer.cpp
        src/implot.cpp
        src/implot_items.cpp
        game/Player.cpp
)
# =========================
# Link libraries
# =========================
target_link_libraries(ProjetM1 PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Network
        OpenGL::GL
        ImGui-SFML::ImGui-SFML

        X11::X11
)