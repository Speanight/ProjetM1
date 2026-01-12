#include <iostream>
#include "communication/Server.hpp"
#include "communication/Client.hpp"
#include "game/Position.hpp"


int main() {
    std::cout << "Starting server on IP: " << SERVER_IP << std::endl;
    Server server;
    Client client;

    std::cout << "Sending position values to server: " << std::endl;
    client.sendData();

    return 0;
}