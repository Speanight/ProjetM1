#include "Client.hpp"

Client::Client(std::chrono::time_point<std::chrono::steady_clock> clock, std::string name, sf::Color color) : server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4) {
    this->packetLoss = 0;
    this->clock = clock;
    this->color = color;
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

unsigned short Client::getPort() {
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

void Client::updateLoop() {
    bool loop = true;
    while (loop) {
        int packetLossChance = std::experimental::randint(1, 100);

        sf::Packet packet;

        // DUMMY VALUES - RANDOM INTS
        Position position(std::experimental::randint(0, 50), std::experimental::randint(0, 50));

        packet << Pkt::POSITION << position;
        std::cout << name << " >>> Server | position: (" << position.getX() << ", " << position.getY() << ")" << std::endl;

        if (packetLossChance > packetLoss) {
            socket.send(packet, server, COMM_PORT_SERVER);
        }
        else {
            std::cout << "Server >X> " << name << " | LOST PACKET!" << std::endl;
        }

        // SLEEP UNTIL NEXT TICK
        clock += TICKRATE;
        std::this_thread::sleep_until(clock);
    }
}
