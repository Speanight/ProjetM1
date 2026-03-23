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
    MainWindow(sf::Clock clock);
    MainWindow(sf::Clock clock, int maxPlayers);
    ~MainWindow();

    void addClient(ClientUI* client);
    Server getServer();

    void draw();
    void loop();

    void createDefaultClients(int count, sf::Clock clock);

};


#endif //PROJETM1_MAINWINDOW_HPP
