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

    sf::Joystick::update();

    for (int i = 0; i < 8; i++) {
        if (sf::Joystick::isConnected(i)) {
            std::cout << "Joystick " << i << " is connected!" << std::endl;
        }
    }

    MainWindow window(clock);

    std::cout << "Tickrate changes " << Const::TICKRATE.count() << " times per second." << std::endl;
    // CONTROLLER MODE = 0 ; KEYBORD MODE = -1
    // TODO : add something that detect if a controller is up to select it
    int ca_controller = -1;
    int cb_controller = -1;

    // TODO : changing the way clients are added (# selection page)
    std::cout << "Starting server on Network: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;

    ClientUI* clientA = new ClientUI(clock, "Client A", ca_controller, sf::Color::Red, 0);
    ClientUI* clientB = new ClientUI(clock, "Client B", cb_controller, sf::Color::Green,0);

    // Settings players keybinds...
    if(ca_controller == -1) {
        // Keyboard keybind for client A:
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
    }
    else {
        // Controller keybind for client A:
        // Axis U = R stick L <-> R
        // Axis V = R stick U <-> D
        // Axis Z = LT button

        clientA->setKeybinds({
         {Inputs::MOVEMENT_DOWN, sf::Joystick::Axis::Y},
         {Inputs::MOVEMENT_RIGHT, sf::Joystick::Axis::X},
         {Inputs::WPN_CCW, 4}, // LB button
         {Inputs::WPN_CW, 5}, // RB button
         {Inputs::WPN_CHANGE, 2},
         {Inputs::ATTACK, sf::Joystick::Axis::R} // RT button
     });
    }

    if(cb_controller == -1) {
        // Keyboard keybind for client B:
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
    }
    else {
        // Controller keybind for client B:

    }

    // TODO : delete this part, the opponent is added with the READY_R reception
    // clientA->addOpponent(clientB->getName(), clientB->getColor());
    // clientB->addOpponent(clientA->getName(), clientA->getColor());
    clientA->init();
    clientB->init();

    std::cout << "Adding client to server..." << std::endl;
    // window.addClient(clientA);
    // window.addClient(clientB);

    // Print window
    sf::ContextSettings settings;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    return 0;
}