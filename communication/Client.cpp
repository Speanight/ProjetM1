//
// Created by Furball on 1/12/2026.
//

#include "Client.hpp"

Client::Client(std::chrono::time_point<std::chrono::steady_clock> clock, std::string name) : server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4) {
    this->clock = clock;
    this->name = std::move(name);
    if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available? - Client" << std::endl;
        this->port = 0;
    }
    else {
        port = socket.getLocalPort();

        thread = std::thread(&Client::updateLoop, this);
    }
}

Client::~Client() {
    if (thread.joinable()) {
        thread.join();
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

void Client::updateLoop() {
    bool loop = true;
    while (loop) {
        sf::Packet packet;

        // DUMMY VALUES - RANDOM INTS
        Position position(std::experimental::randint(0, 50), std::experimental::randint(0, 50));

        packet << Pkt::POSITION << position;

        if (socket.send(packet, server, COMM_PORT_SERVER) == sf::Socket::Status::Done) {
            std::cout << name << " >>> Server | position: (" << position.getX() << ", " << position.getY() << ")" << std::endl;
        }

        // SLEEP UNTIL NEXT TICK
        clock += TICKRATE;
        std::this_thread::sleep_until(clock);
    }
}
