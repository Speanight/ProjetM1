#include <iostream>
#include "communication/Server.hpp"
#include "communication/Client.hpp"
#include "game/Position.hpp"
#include <chrono>
#include <imgui.h>

#include "ui/MainWindow.hpp"
#include "ui/ClientUI.hpp"
#include "ui/ServerUI.hpp"
#include <X11/Xlib.h>

int main() {
    XInitThreads(); // Needed for multi-threading.
    // Initializing objects
    sf::Clock clock;
    // auto clock = std::chrono::steady_clock::now();

    sf::Joystick::update();

    for (int i = 0; i < 8; i++) {
        if (sf::Joystick::isConnected(i)) {
            std::cout << "Joystick " << i << " is connected!" << std::endl;
        }
    }

    MainWindow window(clock);

    std::cout << "Tickrate changes " << Const::TICKRATE.count() << " times per second." << std::endl;

    std::cout << "Starting server on Network: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;
    ClientUI* clientA = new ClientUI(clock, "Client A", sf::Color::Red);
    ClientUI* clientB = new ClientUI(clock, "Client B", sf::Color::Green);

    // Settings players keybinds...
    clientA->setKeybinds({
        {Inputs::MOVEMENT_UP, sf::Keyboard::Key::Z},
        {Inputs::MOVEMENT_DOWN, sf::Keyboard::Key::S},
        {Inputs::MOVEMENT_LEFT, sf::Keyboard::Key::Q},
        {Inputs::MOVEMENT_RIGHT, sf::Keyboard::Key::D},
        {Inputs::WPN_CCW, sf::Keyboard::Key::A},
        {Inputs::WPN_CW, sf::Keyboard::Key::E},
        {Inputs::WPN_CHANGE, sf::Keyboard::Key::W},
        {Inputs::ATTACK, sf::Keyboard::Key::C}
    });

    clientB->setKeybinds({
        {Inputs::MOVEMENT_UP, sf::Keyboard::Key::Up},
        {Inputs::MOVEMENT_DOWN, sf::Keyboard::Key::Down},
        {Inputs::MOVEMENT_LEFT, sf::Keyboard::Key::Left},
        {Inputs::MOVEMENT_RIGHT, sf::Keyboard::Key::Right},
        {Inputs::WPN_CCW, sf::Keyboard::Key::P},
        {Inputs::WPN_CW, sf::Keyboard::Key::M},
        {Inputs::WPN_CHANGE, sf::Keyboard::Key::L},
        {Inputs::ATTACK, sf::Keyboard::Key::O}
    });

    clientA->addOpponent(clientB->getName(), clientB->getColor());
    clientB->addOpponent(clientA->getName(), clientA->getColor());

    std::cout << "Adding client to server..." << std::endl;
    window.addClient(clientA);
    window.addClient(clientB);

    // Print window
    sf::ContextSettings settings;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    return 0;
}