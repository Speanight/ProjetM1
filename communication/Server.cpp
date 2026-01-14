#include "Server.hpp"

Server::Server() {
    if (socket.bind(COMM_PORT_SERVER) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available?" << std::endl;
    }
    else {
        socket.setBlocking(false);
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

int Server::addClient(std::unordered_map<std::string, std::any> infos) {
    if (std::any_cast<bool>(infos["error"])) {
        std::cout << "Error initializing client " << std::any_cast<std::string>(infos["name"]) << std::endl;
        return Err::ERR_CLIENT_INIT;
    }
    clients[std::any_cast<std::string>(infos["name"])] = std::any_cast<unsigned short>(infos["port"]);
    //std::cout << "Added client " << clients["name"] << " on port " << clients["port"] << std::endl;
    return Err::ERR_NONE;
}

int Server::listen() {
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve("127.0.0.1");
    bool loop = true;
    sf::Packet packet;
    short unsigned int port = COMM_PORT_SERVER;
    int type;
    Position position;

    while (loop) {
        sleep(5);
        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {
            std::cout << "SERVER PACKET: ";
            packet >> type;

            switch (type) {
                case Pkt::SHUTDOWN:
                    std::cout << "Received shutdown packet!" << std::endl;
                    loop = false;
                    break;

                default:
                    std::cout << "UNKNOWN PACKET! Type: " << type << std::endl;
            }
        }
        else {
            std::cout << "NOT SERVER PACKET ";
        }

        // Checks for all connected clients:
        for (auto & [name, remotePort] : clients) {
            if (socket.receive(packet, sender, remotePort) == sf::Socket::Status::Done) {
                std::cout << "CLIENT PACKET: ";
                packet >> type;

                switch (type) {
                    case Pkt::POSITION:
                        std::cout << "Received position packet!" << std::endl;
                        packet >> position;

                        std::cout << "Received: (" << position.getX() << ", " << position.getY() << ")" << std::endl;
                        break;

                    default:
                        std::cout << "UNKNOWN CLIENT PACKET! Type: " << type << " from client " << name << std::endl;
                }
            }
        }
    }

    return Err::ERR_NONE; // Exited without any issue.
}

int Server::shutdown() {
//    sf::IpAddress sender = sf::IpAddress::getLocalAddress();
//    sf::Packet packet;
//    short unsigned int port = COMM_PORT_SERVER;
//
//    if (socket.send(packet, sender, port) == sf::Socket::Status::Done) {
//

    return Err::ERR_NONE;
}