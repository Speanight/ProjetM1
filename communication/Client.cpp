#include "Client.hpp"

/**
 * Clients are being created by ClientUI (which extends this class). A client will have all the information needed
 * for the server-client synchronisation, such as packet loss, ping, name (identification), ...
 *
 * @param clock Clock to synchronise with the server. Needed to sync packets!
 * @param name Name given to the client. Can be any string, must be unique!
 * @param color Color given to the client in the Server's console.
 */
Client::Client(std::chrono::time_point<std::chrono::steady_clock> clock, std::string name, sf::Color color) : server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4) {
    this->packetLoss = 0;
    this->clock = clock;
    this->color = color;
    this->ping = 0;
    this->name = std::move(name);
    if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available? - ClientUI" << std::endl;
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

/**
 * Function that returns the result of the init phase of the client. Can be used to get the useful infos of the client,
 * as well as check if it was able to successfully initialize.
 *
 * @return std::map - "error" is true if client couldn't bind to any port. "name" and "port" returns their values.
 */
std::unordered_map<std::string, std::any> Client::init() {
    std::unordered_map<std::string, std::any> infos = {
            {"error", port == 0},
            {"name", name},
            {"port", port},
    };

    return infos;
}

std::string Client::getName() {
    return name;
}

unsigned short Client::getPort() const {
    return port;
}

int Client::getPacketLoss() const {
    return packetLoss;
}

int Client::getPing() const {
    return ping;
}

void Client::setPacketLoss(int packetLoss) {
    this->packetLoss = packetLoss;
}

void Client::setPing(int ping) {
    this->ping = ping;
}

/**
 * Loop that executes every tick rate: client will calculate if packet must be dropped or not, and sends packet of
 * position to the server. This function shouldn't return, except if the server stops and the client should stop
 * executing.
 */
void Client::updateLoop() {
    bool loop = true;
    while (loop) {
        int packetLossChance = std::experimental::randint(1, 100);

        sf::Packet packet;

        // DUMMY VALUES - RANDOM INTS
        Position position(std::experimental::randint(0, 50), std::experimental::randint(0, 50));
        packet << Pkt::POSITION << position;

        if (packetLossChance > packetLoss) {
            socket.send(packet, server, COMM_PORT_SERVER);
        }

        // SLEEP UNTIL NEXT TICK
        clock += TICKRATE;
        std::this_thread::sleep_until(clock);
    }
}
