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
    short screen = 0;

    sf::Clock clock;

public:
//    sf::RenderWindow window;
    MainWindow(sf::Clock clock, bool quickLaunch = false);
    ~MainWindow();

    void addClient(ClientUI* client);
    Server getServer();

    void loop();

    // Screens
    void draw(short screen);
    void drawTitlescreen();
    void drawPlayerSelect();
    void drawGame();

    void quickSetup();
};


#endif //PROJETM1_MAINWINDOW_HPP
