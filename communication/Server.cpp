#include "Server.hpp"

/**
 * A server is being initialized with MainWindow. It's a needed component to ensure communication between the different
 * clients. It receives the packets, and send packets back to each client. For that, they need to be added manually
 * after creation of the server.
 *
 * @param clock Clock, needed to synchronise clients and server together for packet transmission.
 */
Server::Server(const sf::Clock clock) {
    colors = {sf::Color::White, sf::Color::Red, sf::Color::Blue, sf::Color::Green, sf::Color::Yellow};
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

/**
 * Function that allows to get info of clients paired with the server.
 *
 * @return map of clients, keys being their name and values being their port.
 */
std::unordered_map<std::string, unsigned short> Server::getClients() {
    return clients;
}

/**
 * Allows to add clients to the server's tracked routes. This means the server will send and receive packets from the
 * port of the clients added.
 *
 * @param infos map of infos, usually returned by Client::init().
 * @return Error code
 */
int Server::addClient(std::unordered_map<std::string, std::any> infos) {
    if (std::any_cast<bool>(infos["error"])) {
        std::cout << "Error initializing client " << std::any_cast<std::string>(infos["name"]) << std::endl;
        return Err::ERR_CLIENT_INIT;
    }
    clients[std::any_cast<std::string>(infos["name"])] = std::any_cast<unsigned short>(infos["port"]);
    std::cout << "Added client " << std::any_cast<std::string>(infos["name"]) << " on port " << clients[std::any_cast<std::string>(infos["name"])] << std::endl;
    return Err::ERR_NONE;
}

/**
 * Loop that executes every tick rate: Server will calculate position of client if incorrect/impossible. Recovers
 * positions of clients. This function shouldn't return, except if the server stops.
 */
int Server::updateLoop() {
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve("127.0.0.1");
    bool loop = true;
    sf::Packet packet;
    short unsigned int port;
    int type;
    Position position;
    int senderNum;
    const sf::Time time = std::chrono::milliseconds(TICKRATE);

    while (loop) {
        senderNum = 0;
        // TODO: sleep until next tick ONLY FOR SENDING DATA!
        // sf::sleep(time);

        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {
            if (port == COMM_PORT_SERVER) {
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
                senderNum++;
                if (remotePort == port) { // Check if ports corresponds (AKA the expected client)
                    packet >> type;

                    switch (type) {
                        case Pkt::POSITION:
                            int time;
                            packet >> time >> position;
                            std::cout << "Received packet @: " << time << " | Server is @:" << clock.getElapsedTime().asMilliseconds() << std::endl;
                            std::cout << "This means a delay of: " << clock.getElapsedTime().asMilliseconds() - time << " ms" << std::endl;
                            std::cout << "|------------------------------------|" << std::endl;

                            addLine(name + " >>> Server [PING:" + std::to_string(clock.getElapsedTime().asMilliseconds() - time) + "ms] | position: (" + std::to_string(position.getX()) + ", " + std::to_string(position.getY()) + ")", colors[senderNum]);
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

/**
 * Sends a shutdown packet to the server as well as the clients. Useful to stop the execution of the whole script
 * gracefully.
 *
 * @return Error code
 */
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
