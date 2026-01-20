cmake_minimum_required(VERSION 3.22)
project(ProjetM1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================
# SFML 3
# =========================
find_package(SFML 3 REQUIRED COMPONENTS
        Window
        System
)

# =========================
# OpenGL (fourni par le système / SFML)
# =========================
find_package(OpenGL REQUIRED)

# =========================
# ImGui (local)
# =========================
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/ressources/imgui)

set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        # ${IMGUI_DIR}/imgui_demo.cpp   # optionnel
)

# =========================
# Executable
# =========================
add_executable(ProjetM1
        main.cpp
        ${IMGUI_SOURCES}
)

# =========================
# Includes
# =========================
target_include_directories(ProjetM1 PRIVATE
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

# =========================
# Link libraries
# =========================
target_link_libraries(ProjetM1 PRIVATE
        SFML::Window
        SFML::System
        OpenGL::GL
)
