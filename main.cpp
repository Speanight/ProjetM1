#include <iostream>
#include "communication/Server.hpp"
#include "communication/Client.hpp"
#include "game/Position.hpp"
#include <chrono>
#include <thread>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics.hpp>


int main() {
    // Clock will be used to sync clients, server, and refresh times (packets travels)
    auto clock = std::chrono::steady_clock::now();

    std::cout << "Starting server on IP: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;
    Server server(clock);
    Client client(clock, "Client A");

    std::cout << "Adding client to server..." << std::endl;
    server.addClient(client.init());


    sf::RenderWindow window(sf::VideoMode({640, 480}), "Projet M1");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    sf::Clock deltaClock;
    while (window.isOpen()) {
        //////////////////////
        // TEMP - TO UPDATE //
        //////////////////////
        int clientPacketLoss = client.getPacketLoss();

        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin((client.getName() + "'s settings").c_str());
        ImGui::Text(("Port: " + std::to_string(client.getPort())).c_str());
        ImGui::SliderInt("Packet loss (%)", &clientPacketLoss, 0, 100);
        client.setPacketLoss(clientPacketLoss);
        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    if (server.shutdown() == Err::ERR_SERVER_SHUTDOWN) {
        std::cout << "Server failed to shut down! Calling destructors as a backup solution..." << std::endl;
        server.~Server();
        client.~Client();
    }

    std::cout << "Sent server shutdown!" << std::endl;

    return 0;
}