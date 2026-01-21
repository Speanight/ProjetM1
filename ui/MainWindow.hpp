#ifndef PROJETM1_MAINWINDOW_HPP
#define PROJETM1_MAINWINDOW_HPP

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include "ServerUI.hpp"


class MainWindow {
private:
    ServerUI server;
public:
    MainWindow();

    void draw();
    void loop();

};


#endif //PROJETM1_MAINWINDOW_HPP
