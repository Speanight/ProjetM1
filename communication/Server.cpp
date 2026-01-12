#include <optional>
#include "Server.hpp"
Server::Server() {
    if (socket.bind(COMM_PORT) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available?" << std::endl;
    }
    else {
        thread = std::thread([this] {
            this->listen();
        });
    }
}

Server::~Server() {
    if (thread.joinable()) {
        thread.join();
    }
}

int Server::listen() {
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve("127.0.0.1");
    bool loop = true;
    sf::Packet packet;
    short unsigned int port = COMM_PORT;

        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {
            Position position;
            packet >> position;

            std::cout << "Received: (" << position.getX() << "; " << position.getY() << ")" << std::endl;
        }

    return ERR_NONE; // Exited without any issue.
}