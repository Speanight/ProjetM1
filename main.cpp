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

#include "ui/Console.hpp"


int main() {
    // Clock will be used to sync clients, server, and refresh times (packets travels)
    auto clock = std::chrono::steady_clock::now();

    std::cout << "Starting server on IP: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;
    Server server(clock);
    Client client(clock, "Client A");

    std::cout << "Adding client to server..." << std::endl;
    server.addClient(client.init());


    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Projet M1");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    sf::View gameView;
    sf::View serverView;

    gameView.setViewport(sf::FloatRect({0.f, 0.f}, {0.7f, 1.f}));
    serverView.setViewport(sf::FloatRect({0.f, 0.7f}, {0.3f, 1.f}));

    Console console("../arial.ttf", sf::Vector2f (0, 0));
    console.addLine("Test1");
    console.addLine("Test2");
    console.addLine("Test3", sf::Color::Red);

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    sf::CircleShape shape2(80.f);
    shape.setFillColor(sf::Color::Blue);

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
        ImGui::Text("%s", ("Port: " + std::to_string(client.getPort())).c_str());
        ImGui::SliderInt("Packet loss (%)", &clientPacketLoss, 0, 100);
        client.setPacketLoss(clientPacketLoss);
        ImGui::End();

        window.clear(); // Clears the past screen.
        window.setView(gameView);
        window.draw(shape);
        window.setView(serverView);
        console.draw(window);
        window.setView(window.getDefaultView());
        ImGui::SFML::Render(window);
        window.display(); // Display the new screen
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