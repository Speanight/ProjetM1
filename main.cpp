#include <iostream>
#include "communication/Server.hpp"
#include "communication/Client.hpp"
#include "game/Position.hpp"


int main() {
    std::cout << "Starting server on IP: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;
    Server server;
    Client client("Client A");

    std::cout << "Adding client to server..." << std::endl;
    // TODO: Fix: client et port = 0
    server.addClient(client.init());

    std::cout << "Sending position values to server: " << std::endl;
    client.sendData();
    client.sendData();
    client.sendData();
    client.sendData();
    client.sendData();



    return 0;
}