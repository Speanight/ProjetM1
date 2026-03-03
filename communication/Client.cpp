#include "Client.hpp"

#include <utility>
#include <SFML/Window/Keyboard.hpp>

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

int Client::getPacketLoss() const {
    return packetLoss;
}

int Client::getPing() const {
    return ping;
}

Position Client::getPosition() {
    return position;
}

void Client::setPacketLoss(int packetLoss) {
    this->packetLoss = packetLoss;
}

void Client::setPing(int ping) {
    this->ping = ping;
}

///////////////
// FUNCTIONS //
///////////////
void Client::move(ImVec2 direction, float deltaTime) {
    position.setX(position.getX() + direction.x * Const::PLAYER_SPEED * deltaTime);
    position.setY(position.getY() + direction.y * Const::PLAYER_SPEED * deltaTime);
}

void Client::clampToChild(ImVec2 childMin, ImVec2 childMax) {
    if (position.getX() - m_radius < childMin.x)
        position.setX(childMin.x + m_radius);

    if (position.getX() + m_radius > childMax.x)
        position.setX(childMax.x - m_radius);

    if (position.getY() - m_radius < childMin.y)
        position.setY(childMin.y + m_radius);

    if (position.getY() + m_radius > childMax.y)
        position.setY(childMax.y - m_radius);
}

void Client::resolveCollision(Client& other) {
    ImVec2 diff = { other.getPosition().getX() - position.getX(),
                    other.getPosition().getY() - position.getY()};

    float distance = sqrtf(diff.x*diff.x + diff.y*diff.y);
    float minDistance = m_radius + other.m_radius;

    if (distance < minDistance)
    {
        if (distance == 0.f)
        {
            diff = {1.f, 0.f};
            distance = 1.f;
        }

        ImVec2 normal = { diff.x / distance, diff.y / distance };
        float penetration = minDistance - distance;

        other.getPosition().setX(other.getPosition().getX() + normal.x * penetration);
        other.getPosition().setY(other.getPosition().getY() + normal.y * penetration);
    }
}


Input Client::getInputs() {
    Input input;
    for (const std::pair<const int, sf::Keyboard::Key> & i : keybinds) {
        input.handleInput(i.first, isKeyPressed(i.second));
    }

    return input;
}

void Client::setKeybinds(std::unordered_map<int, sf::Keyboard::Key> keybinds) {
    this->keybinds = std::move(keybinds);
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

        QueuedPacket pkt;
        pkt.timestamp = clock.getElapsedTime();
        auto inputs = getInputs();
        pkt.packet << Pkt::POSITION << pkt.timestamp.asMilliseconds() << position << inputs;
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

//                        position.setX(tempPos.getX());
//                        position.setY(tempPos.getY());
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

sf::Color Client::getColor() {
    return this->color;
}

Client::Client(const Client& other) : server(other.server) {
    this->name = other.name;
    this->color = other.color;
    this->position = Position(other.position.getX(), other.position.getY());
}

Client& Client::operator=(const Client& other) {
    this->color = other.color;
    this->clock = other.clock;
    this->name = other.name;

    return *this;
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