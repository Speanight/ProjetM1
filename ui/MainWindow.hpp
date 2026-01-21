#ifndef PROJETM1_MAINWINDOW_HPP
#define PROJETM1_MAINWINDOW_HPP

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <vector>
#include <thread>

#include "ClientUI.hpp"
#include "../communication/Server.hpp"


class MainWindow {
private:
    std::thread thread;
    Server server;
    std::vector<ClientUI*> clients;
public:
    MainWindow(std::chrono::time_point<std::chrono::steady_clock> clock);
    ~MainWindow();

    void addClient(ClientUI* client);
    Server getServer();

    void draw();
    void loop();

};


#endif //PROJETM1_MAINWINDOW_HPP
