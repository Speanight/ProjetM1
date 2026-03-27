#include <iostream>
#include "ui/MainWindow.hpp"
#include <X11/Xlib.h> // Must be included AFTER SFML, aka AFTER MainWindow!
#include "Utils.hpp"

unsigned short tickrate;
unsigned short clientRefreshRate;

int main() {
    XInitThreads(); // Needed for multi-threading.
    sf::Clock clock;

    tickrate = 10;
    clientRefreshRate = 200;

    std::cout << "Tickrate changes " << tickrate << " times per second." << std::endl;
    std::cout << "Starting server on Network: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;

    MainWindow window(clock, true);

    return 0;
}