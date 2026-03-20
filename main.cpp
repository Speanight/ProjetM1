#include <iostream>
#include "ui/MainWindow.hpp"
#include <X11/Xlib.h> // Must be included AFTER SFML, aka AFTER MainWindow!
#include "Utils.hpp"

int main() {
    XInitThreads(); // Needed for multi-threading.
    sf::Clock clock;

    std::cout << "Tickrate changes " << Const::TICKRATE.count() << " times per second." << std::endl;
    std::cout << "Starting server on Network: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;

    MainWindow window(clock, true);

//    // Print window
//    sf::ContextSettings settings;
//    settings.majorVersion = 3;
//    settings.minorVersion = 3;

    return 0;
}