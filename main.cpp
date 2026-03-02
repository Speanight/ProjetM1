#include <iostream>
#include "communication/Server.hpp"
#include "communication/Client.hpp"
#include "game/Position.hpp"

#include "ui/MainWindow.hpp"
#include "ui/ClientUI.hpp"
#include "ui/ServerUI.hpp"

struct NetConfig {
    enum class CompensationMode {
        COMPO_1 , COMPO_2, COMPO_3, MODE_1, MODE_2, None
    } compensation = CompensationMode::None;
};

const char* toString(NetConfig::CompensationMode mode) {
    switch (mode) {
        case NetConfig::CompensationMode::None:    return "None";
        case NetConfig::CompensationMode::COMPO_1: return "COMPO_1";
        case NetConfig::CompensationMode::COMPO_2: return "COMPO_2";
        case NetConfig::CompensationMode::COMPO_3: return "COMPO_3";
        case NetConfig::CompensationMode::MODE_1:  return "MODE_1";
        case NetConfig::CompensationMode::MODE_2:  return "MODE_2";
        default: return "Unknown";
    }
}

int main() {
    // Initializing objects
    sf::Clock clock;
    // auto clock = std::chrono::steady_clock::now();

    MainWindow window(clock);

    std::cout << "Starting server on Network: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;
    ClientUI* clientA = new ClientUI(clock, "Client A");
    ClientUI* clientB = new ClientUI(clock, "Client B");

    std::cout << "Adding client to server..." << std::endl;
    window.addClient(clientA);
    window.addClient(clientB);

    // Print window
    sf::ContextSettings settings;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    return 0;
}