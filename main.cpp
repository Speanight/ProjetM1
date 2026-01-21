#include <iostream>
#include "communication/Server.hpp"
#include "communication/Client.hpp"
#include "game/Position.hpp"
#include <chrono>
#include <imgui.h>

#include "ui/MainWindow.hpp"
#include "ui/ClientUI.hpp"
#include "ui/ServerUI.hpp"

#include "ui/ServerUI.hpp"

struct NetConfig {
    // Default configuration
    int packetLossGame1 = 0;
    int pingGame1 = 50;

    int packetLossGame2 = 0;
    int pingGame2 = 50;

    int tickrate = 60;

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

void onNetConfigChanged(const NetConfig& config) {
    std::cout << "Changement dans la config\n";

    std::cout << "Game 1:\n";
    std::cout << "  Packet loss: " << config.packetLossGame1 << "%\n";
    std::cout << "  Ping: " << config.pingGame1 << " ms\n";

    std::cout << "Game 2:\n";
    std::cout << "  Packet loss: " << config.packetLossGame2 << "%\n";
    std::cout << "  Ping: " << config.pingGame2 << " ms\n";

    std::cout << "Global:\n";
    std::cout << "  Tickrate: " << config.tickrate << "\n";
    std::cout << "  Compensation: "
              << toString(config.compensation) << "\n";

    std::cout << "=============================\n\n";
}



void drawGameZone(const char* title) {}

bool drawGameConfig(const char* title, int& packetLoss, int& ping) {}

bool drawGlobalConfig(NetConfig& config) {
    bool changed = false;

    const char* modes[] = { "COMPO 1 ", "COMPO 2", "COMPO 3", "MODE 1", "MODE 2" };
    int current = static_cast<int>(config.compensation);

    changed |= ImGui::Combo("Compensation", &current, modes, IM_ARRAYSIZE(modes));
    config.compensation = static_cast<NetConfig::CompensationMode>(current);

    changed |= ImGui::InputInt("Tickrate", &config.tickrate);

    return changed;
}

void drawServerZone() {}

void drawMainUI(NetConfig& config) {}


int main() {
    // Initializing objects
    auto clock = std::chrono::steady_clock::now();

    MainWindow window(clock);

    std::cout << "Starting server on IP: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;
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