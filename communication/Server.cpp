#include "Server.hpp"

Server::Server(const std::chrono::time_point<std::chrono::steady_clock> clock) {
    this->clock = clock;
    if (socket.bind(COMM_PORT_SERVER) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available?" << std::endl;
    }
    else {
        socket.setBlocking(true);
        thread = std::thread(&Server::updateLoop, this);
    }
}

Server::~Server() {
    if (thread.joinable()) {
        thread.join();
    }
}

std::unordered_map<std::string, unsigned short> Server::getClients() {
    return clients;
}

int Server::addClient(std::unordered_map<std::string, std::any> infos) {
    if (std::any_cast<bool>(infos["error"])) {
        std::cout << "Error initializing client " << std::any_cast<std::string>(infos["name"]) << std::endl;
        return Err::ERR_CLIENT_INIT;
    }
    clients[std::any_cast<std::string>(infos["name"])] = std::any_cast<unsigned short>(infos["port"]);
    std::cout << "Added client " << std::any_cast<std::string>(infos["name"]) << " on port " << clients[std::any_cast<std::string>(infos["name"])] << std::endl;
    return Err::ERR_NONE;
}

int Server::updateLoop() {
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve("127.0.0.1");
    bool loop = true;
    sf::Packet packet;
    short unsigned int port;
    int type;
    Position position;

    while (loop) {
        // SLEEP UNTIL NEXT TICK
        clock += TICKRATE;
        std::this_thread::sleep_until(clock);

        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {
            std::cout << "Server >>> ";
            if (port == COMM_PORT_SERVER) {
                std::cout << "SERVER PACKET: ";
                packet >> type;

                switch (type) {
                    case Pkt::SHUTDOWN:
                        std::cout << "Received shutdown packet!" << std::endl;
                        // TODO: Send server shutdown packet to clients!
                        loop = false;
                        break;

                    default:
                        std::cout << "UNKNOWN PACKET! Type: " << type << std::endl;
                }
            }

            // Checks for all connected clients:
            for (auto & [name, remotePort] : clients) {
                if (remotePort == port) { // Check if ports corresponds (AKA the expected client)
                    std::cout << name << " ";
                    packet >> type;

                    switch (type) {
                        case Pkt::POSITION:
                            packet >> position;

                            std::cout << " | position: (" << position.getX() << ", " << position.getY() << ")" << std::endl;
                            break;

                        default:
                            std::cout << " UNKNOWN CLIENT PACKET! Type: " << type << " from client " << name << std::endl;
                    }
                }
            }
        }
    }

    socket.unbind();
    thread.join();
    return Err::ERR_NONE; // Exited without any issue.
}

int Server::shutdown() {
    sf::Packet packet;
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve("127.0.0.1");

    packet << Pkt::SHUTDOWN;

    if (socket.bind(COMM_PORT_SERVER) == sf::Socket::Status::Done) {
        if (socket.send(packet, sender.value(), COMM_PORT_SERVER) == sf::Socket::Status::Done) {
            // Sends DC packet to all clients
            for (auto & [name, remotePort] : clients) {
                socket.send(packet, sender.value(), remotePort);
            }
            return Err::ERR_NONE;
        }
    }

    return Err::ERR_SERVER_SHUTDOWN;
}