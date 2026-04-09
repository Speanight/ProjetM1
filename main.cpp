#include <iostream>
#include "ui/MainWindow.hpp"
#include <X11/Xlib.h> // Must be included AFTER SFML, aka AFTER MainWindow!
#include "Utils.hpp"

// Global variables definition
unsigned short tickrate;
unsigned short clientRefreshRate;
uint32_t packetID;
std::mutex m;
std::vector<sf::Texture> textureMaps;

int main() {
    XInitThreads(); // Needed for multi-threading.
    sf::Clock clock;

    tickrate = 5;
    clientRefreshRate = 10;
    packetID = 0;

    // Define maps to avoid drawing them constantly:
//    for (auto& i : Const::MAP_LINK) {
//        sf::Texture texture;
//        if (texture.loadFromFile(i)) {
//            textureMaps.push_back(texture);
//        }
//    }

    std::cout << "Tickrate changes " << tickrate << " times per second." << std::endl;
    std::cout << "Starting server on Network: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;

    MainWindow window(clock, false);

    return 0;
}