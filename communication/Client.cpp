//
// Created by Furball on 1/12/2026.
//

#include "Client.hpp"

#include <iostream>
//#include <utility>

Client::Client(std::string name) : server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4) {
    this->name = std::move(name);
    if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available? - Client" << std::endl;
        this->port = 0;
    }
    else {
        port = socket.getLocalPort();
    }
}

std::unordered_map<std::string, std::any> Client::init() {
    std::unordered_map<std::string, std::any> infos = {
            {"error", port == 0},
            {"name", name},
            {"port", port},
    };

    return infos;
}

void Client::sendData() {
    // DUMMY VALUES
    sf::Packet packet;
    Position position(5, 6);

    packet << Pkt::POSITION << position;

    if (socket.send(packet, server, COMM_PORT_SERVER) == sf::Socket::Status::Done) {
        std::cout << "Sent packet!" << std::endl;
    }
}
