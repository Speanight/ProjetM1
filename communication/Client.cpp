//
// Created by Furball on 1/12/2026.
//

#include "Client.hpp"

#include <iostream>

Client::Client() : server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4) {
    if (socket.bind(COMM_PORT) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available? - Client" << std::endl;
    }
}

void Client::sendData() {
    // DUMMY VALUES
    sf::Packet packet;
    Position position(5, 6);

    packet << position;

    if (socket.send(packet, server, COMM_PORT) == sf::Socket::Status::Done) {
        std::cout << "Sent packet!" << std::endl;
    }
}
