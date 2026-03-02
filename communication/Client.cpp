#include "Client.hpp"

/**
 * Clients are being created by ClientUI (which extends this class). A client will have all the information needed
 * for the server-client synchronisation, such as packet loss, ping, name (identification), ...
 *
 * @param clock Clock to synchronise with the server. Needed to sync packets!
 * @param name Name given to the client. Can be any string, must be unique!
 * @param color Color given to the client in the Server's console.
 */
Client::Client(const sf::Clock clock, std::string name, sf::Color color) : server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4) {
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
        std::cout << "Client " << this->name << " started on port: " << port << std::endl;

        socket.setBlocking(true);

        sendThread = std::thread(&Client::sendLoop, this);
        receiveThread = std::thread(&Client::receiveLoop, this);
    }
}

Client::~Client() {
    socket.unbind();
    if (sendThread.joinable()) {
        sendThread.join();
    }
    if (receiveThread.joinable()) {
        receiveThread.join();
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
int Client::sendLoop() {
    const sf::Time time = std::chrono::milliseconds(TICKRATE);
    while (loop) {
        int packetLossChance = std::experimental::randint(1, 100);

        // DUMMY VALUES - RANDOM INTS
        Position position(std::experimental::randint(0, 50), std::experimental::randint(0, 50));
        QueuedPacket pkt;
        pkt.timestamp = clock.getElapsedTime();
        std::unordered_map<int,int> inputs = {{Inputs::MOVEMENT_X, 1}};
        // TODO: add inputs to packet and add packet << / >> function to unordered_map
        pkt.packet << Pkt::POSITION << pkt.timestamp.asMilliseconds() << position;
        packets.push_back(pkt); // Adds the packet to the array of packets.

        // If the packet isn't lost (editable through packet loss % slider):
        if (packetLossChance > packetLoss) {
            auto packet = getLatestPacket(); // Get latest packet that meets ping criteria.

            // Check that packet does exist:
            if (packet.has_value()) {
                socket.send(packet.value(), server, COMM_PORT_SERVER);
            }
        }

        // SLEEP UNTIL NEXT TICK
        sf::sleep(time);
    }

    return Err::ERR_NONE;
}

int Client::receiveLoop() {
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve(SERVER_IP);
    sf::Packet packet;
    int type;
    const sf::Time tickrate = std::chrono::milliseconds(TICKRATE);
    short unsigned int port;

    while (loop) {
        sf::sleep(sf::Time()); // "empty" sleep: required for loops.
        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {
            if (port == COMM_PORT_SERVER) {
                packet >> type;

                switch (type) {
                    case Pkt::SHUTDOWN:
                        std::cout << "Client " << name << " received shutdown packet!" << std::endl;
                        loop = false;
                        break;

                    case Pkt::POSITION: {
                        Position tempPos;
                        packet >> tempPos;

                        position.setX(tempPos.getX());
                        position.setY(tempPos.getY());
                        // TODO: Handle position reception
                        break;
                    }

                    default:
                        std::cout << "UNKNOWN PACKET! Type: " << type << " for client " << name << std::endl;
                }
            }
        }
    }

    sendThread.join();

    return Err::ERR_NONE;
}

/**
 * Function that returns the latest packet. This will go through the packets vector of the Client object, will check for
 * ping value, and will return the last packet respecting the given context. If no packet matches the criteria, an
 * empty packet will be returned.
 * Please note that this will alter the packets deque!
 *
 * @return Latest packet respecting ping value. Empty packet if none corresponds.
 */
std::optional<sf::Packet> Client::getLatestPacket() {
    // Shouldn't happen: only if packets is empty.
    if (packets.empty()) {
        return std::nullopt;
    }

    sf::Time now = clock.getElapsedTime();
    sf::Time timestamp = now - sf::milliseconds(ping);

    sf::Packet toSend;
    bool found = false;

    // Check in the packets until we find the right one.
    while (!packets.empty() && packets.front().timestamp <= timestamp) {
        toSend = packets.front().packet;
        packets.pop_front();
        found = true;
    }

    // Return the corresponding packet
    if (found) {
        return toSend;
    }

    // Shouldn't happen: error case.
    return std::nullopt;
}