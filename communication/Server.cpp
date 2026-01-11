#include <optional>
#include "Server.hpp"
Server::Server() {
    if (socket.bind(COMM_PORT) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available?" << std::endl;
    }
    else {
        thread = std::thread([this] {
            this->listen((const sf::IpAddress &) SERVER_IP);
        });
    }
}

Server::~Server() {
    if (thread.joinable()) {
        thread.join();
    }
}

int Server::listen(sf::IpAddress sende) {
    std::optional<sf::IpAddress> sender;
    //sf::IpAddress sender(127, 0, 0, 1);
    bool loop = true;
    sf::Packet packet;
    short unsigned int port = COMM_PORT;

        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {

        }

    return ERR_NONE; // Exited without any issue.
}