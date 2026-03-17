#include "Client.hpp"

#include <algorithm>
#include <utility>
#include <bits/ranges_algo.h>
#include <SFML/Window/Keyboard.hpp>

/**
 * Clients are being created by ClientUI (which extends this class). A client will have all the information needed
 * for the server-client synchronisation, such as packet loss, ping, name (identification), ...
 *
 * @param clock Clock to synchronise with the server. Needed to sync packets!
 * @param name Name given to the client. Can be any string, must be unique!
 * @param color Color given to the client in the Server's console.
 */
Client::Client(const sf::Clock clock, std::string name, sf::Color color) : server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4), semaphore(1) {
    this->network.packetLoss = 0;
    this->clock = clock;
    this->network.ping = 0;
    this->player.name = std::move(name);
    this->player.color = color;
    this->bufferOnReceipt.addClient(player);
    this->player.radius = std::numbers::pi/2; //put the element on top in radius
    this->player.mode = true;
    this->player.isAttacking = false;
    this->player.timer_atk = -1;

    Weapon wpn (0);                         // ID of the default wpn
    this->player.wpn = wpn;
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
        this->player.port = 0;
    }
    else {
        player.port = socket.getLocalPort();
        std::cout << "Client " << this->player.name << " started on port: " << player.port << std::endl;

        socket.setBlocking(true);

        sendThread = std::thread(&Client::update, this);
        receiveThread = std::thread(&Client::receiveLoop, this);
    }

    std::unordered_map<std::string, std::any> infos = {
            {"error", player.port == 0},
            {"name", player.name},
            {"port", player.port},
            {"color", player.color}
    };

    return infos;
}

Player Client::getPlayer() {
    return player;
}

std::string Client::getName() {
    return player.name;
}

int Client::getPacketLoss() const {
    return network.packetLoss;
}

int Client::getPing() const {
    return network.ping;
}

float Client::getRadius() const {
    return this->player.radius;
}

Position Client::getPosition() const {
    return this->player.position;
}

std::array<bool,3> Client::getCompensations() const {
    return network.compensations;
}

bool Client::getCompensationEnabled(int compensation) {
    if (compensation < network.compensations.size()) {
        return network.compensations[compensation];
    }
    return false;
}

void Client::setPosition(Position p) {
    this->player.position.setX(p.getX());
    this->player.position.setY(p.getY());
}

void Client::setRadius(float radius) {
    this->player.radius = std::fmod(radius, 2.f * std::numbers::pi);
}

void Client::setPacketLoss(int packetLoss) {
    if (packetLoss >= 0 and packetLoss <= 100) {
        this->network.packetLoss = packetLoss;
    }
}

void Client::setPing(int ping) {
    if (ping >= 0) {
        this->network.ping = ping;
    }
}

void Client::setCompensations(std::array<bool,3> compensations) {
    this->network.compensations = compensations;
}

void Client::setKeybinds(std::unordered_map<int, sf::Keyboard::Key> keybinds) {
    this->keybinds = std::move(keybinds);
}

/**
 * Function that recovers all the users inputs. This uses user's defined keybinds and iterates
 * over each of them to construct an object called "Input". This function calls a function named
 * "handleInput", from the object "Input", which will put the correct key/value into the correct
 * variable in the object Input.
 *
 * @param mode_enable (<optional> false by default) - Allows you to give the function last weapon mode value (attack/defense)
 * @param attack_enable (<optional> true by default) - Allows the user to attack (should be false if cooldown, switching weapon, ...)
 * @return - Returns an object Input with corresponding values according to keys pressed.
 */
Input Client::getInputs(bool mode_enable, bool attack_enable) {
    Input input;

    for (const std::pair<const int, sf::Keyboard::Key> & i : keybinds) {
        bool pressed = isKeyPressed(i.second) > 0.f;
        switch (i.first) {
            case Inputs::WPN_CW :
                if(this->player.timer_atk == -1) {
                    input.handleInput(i.first, isKeyPressed(i.second));
                    break;
                }
            case Inputs::WPN_CCW :
                if(this->player.timer_atk == -1) {
                    input.handleInput(i.first, isKeyPressed(i.second));
                    break;
                }
            case Inputs::WPN_CHANGE:
                if(pressed && !mode_enable && this->player.timer_atk == -1) {
                    input.setMode(true);
                }

                input.setModeEnable(pressed);
                break;
            case Inputs::ATTACK:
                if(pressed && !attack_enable && this->player.timer_atk == -1) {
                    input.setAttack(true);
                    // printf("Attack click !\n");
                }
                input.setAttackEnable(pressed);
                break;
            default :
                input.handleInput(i.first, isKeyPressed(i.second));
                break;
        }

    }

    return input;
}

/**
 * @brief Loops that executes at every frame. Gather inputs, send packet and applies compensations.
 * @return - Error code. (Const. Err:: in Utils.hpp)
 */
int Client::update() {
    // Init with a round start:
    sf::Packet packet;
    packet << Pkt::ROUND_START;
    socket.send(packet, server, COMM_PORT_SERVER);

    bool mode_enable = true;    // set the ability to change the weapon to true at the beginning
    bool attack_enable = true;  // set the ability to attack at true at the beginning

    int before = 0;

    while (loop) {
        // ==========| INPUTS |========== //
        Input inputs = this->getInputs(mode_enable, attack_enable);
        mode_enable = inputs.getModeEnable();
        attack_enable = inputs.getAttackEnable();
        inputs.setWpnID(this->player.wpn.getId());

        // Storing recent local positions to re-adjust if needed.
        State state(clock.getElapsedTime().asMilliseconds(), getPlayer().position, getRadius(), getPlayer().mode, getPlayer().isAttacking, getPlayer().wpn.getId(), inputs);
        inputs.setId(lastInputId);
        inputsBuffer[lastInputId] = state;
        lastInputId++;

        // ==========| PACKET HANDLER |========== //
        // Verifying if we should drop packet (packet loss %):
        int packetLossChance = std::experimental::randint(1, 100);
        if (packetLossChance > network.packetLoss) {
            sendPacket(inputs);
        }

        // ==========| COMPENSATIONS (if needed) |========== //
        semaphore.acquire();
        auto compensations = network.compensations;
        if (getCompensationEnabled(Compensation::INTERPOLATION)) {
            compensationInterpolation();
        }
        if (getCompensationEnabled(Compensation::PREDICTION)) {
            compensationPrediction(inputs);
        }
        if (getCompensationEnabled(Compensation::RECONCILIATION)) {
            compensationReconciliation();
        }
        semaphore.release();

        lastUpdate = clock.getElapsedTime().asMilliseconds();

        // ===== ATTACK =====
        if(player.isAttacking && player.timer_atk == -1) {
            // printf("Player is attacking\n");
            player.timer_atk = lastUpdate - before;
        }
        if(player.timer_atk != -1) {
            player.timer_atk += lastUpdate - before;
            // printf("progress : dt = %d\n", player.timer_atk);
        }
        if(player.timer_atk >= player.wpn.getAttackSpeed() + player.wpn.getReload()) {
            // printf("Retour a -1 !\n");
            player.timer_atk = -1;
        }

        for(auto& [name, opp] : opponents) {
            if(opp.isAttacking && opp.timer_atk == -1) {
                // printf("Player is attacking\n");
                opp.timer_atk = lastUpdate - before;
            }
            if(opp.timer_atk != -1) {
                opp.timer_atk += lastUpdate - before;
                // printf("progress : dt = %d\n", opp.timer_atk);
            }
            if(opp.timer_atk >= opp.wpn.getAttackSpeed() + opp.wpn.getReload()) {
                // printf("Retour a -1 !\n");
                opp.timer_atk = -1;
            }
        }
        before = lastUpdate;
        sf::sleep(sf::Time()); // Shortest sleep possible (as update loop runs as fast as possible)
    }

    return Err::ERR_NONE;
}

/**
 * Loop that executes every tick rate: client will calculate if packet must be dropped or not, and sends packet of
 * position to the server. This function shouldn't return, except if the server stops and the client should stop
 * executing.
 */
int Client::sendPacket(Input inputs) {
    if (newGame) {
        sf::Packet packet;
        packet << Pkt::ACK << Pkt::ROUND_START;

        socket.send(packet, server, COMM_PORT_SERVER);

        newGame = false;
    }

    QueuedPacket pkt;
    pkt.timestamp = clock.getElapsedTime();

    pkt.packet << Pkt::INPUTS << pkt.timestamp.asMilliseconds() << inputs;
    packets.push_back(pkt); // Adds the packet to the array of packets.

    // If the packet isn't lost (editable through packet loss % slider):
    auto packet = getLatestPacket(); // Get latest packet that meets ping criteria.

    // Check that packet does exist:
    if (packet.has_value()) {
        socket.send(packet.value(), server, COMM_PORT_SERVER);
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
        packet.clear();
        sf::sleep(sf::Time()); // "empty" sleep: required for loops.
        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {
            if (port == COMM_PORT_SERVER) {
                packet >> type;

                switch (type) {
                    case Pkt::ROUND_START:
                        std::cout << "Client " << getName() << " received ROUND_START" << std::endl;
                        newGame = true;
                        // No break because round start has position afterwards (and therefor will execute Pkt::GLOBAL case)

                    case Pkt::GLOBAL: {
                        int nbPlayers;
                        int stateTick;
                        std::string name;
                        packet >> stateTick >> nbPlayers;

                        while (nbPlayers > 0) {
                            State state;
                            packet >> name >> state;
                            std::unordered_map<std::string, State> currentState = bufferOnReceipt.getCurrentState();
                            std::unordered_map<std::string, State> pastState = bufferOnReceipt.getTState(-1);

                            if (name == this->getName()) {
                                this->bufferOnReceipt.updateNextPlayerState(player, state);
                                semaphore.acquire();
                                State currState = bufferOnReceipt.getLastState(player);
                                semaphore.release();

                                if (!this->getCompensations()[Compensation::RECONCILIATION]) {
                                    this->player.position.setX(currState.getPosition().getX());
                                    this->player.position.setY(currState.getPosition().getY());
                                    this->player.radius = state.getRadius();
                                }
                                this->player.mode = state.getMode();
                                this->player.isAttacking = state.getAttack();
                                this->player.wpn.applyID(state.getWpn().getId());
                                this->player.point = state.getPoint();
                            }
                            else {
                                // Opponent position:
                                this->bufferOnReceipt.updateNextPlayerState(opponents[name], state);
                                opponents[name].position.setX(currentState[name].getPosition().getX());
                                opponents[name].position.setY(currentState[name].getPosition().getY());
                                opponents[name].radius = currentState[name].getRadius();
                                opponents[name].mode = currentState[name].getMode();
                                opponents[name].isAttacking = currentState[name].getAttack();
                                opponents[name].wpn.applyID(currentState[name].getWpn().getId());
                                opponents[name].point = currentState[name].getPoint();
                            }
                            nbPlayers--;
                        }

                        this->bufferOnReceipt.push(stateTick);
                        packet >> lastServerTick;

                        break;
                    }

                    case Pkt::SHUTDOWN:
                        std::cout << "Client " << player.name << " received shutdown packet!" << std::endl;
                        loop = false;
                        break;

                    case Pkt::POSITION: {
                        Position tempPos;
                        packet >> tempPos;
                        break;
                    }

                    default:
                        std::cout << "UNKNOWN PACKET! Type: " << type << " for client " << player.name << std::endl;
                }
            }
        }
    }

    sendThread.join();

    return Err::ERR_NONE;
}

sf::Color Client::getColor() {
    return this->player.color;
}

// Copy constructors
Client::Client(const Client& other) : server(other.server), semaphore(1) {
    this->player.name = other.player.name;
    this->player.color = other.player.color;
    this->player.position = Position(other.player.position.getX(), other.player.position.getY());
    this->player.radius = other.player.radius;
    this->player.mode = other.player.mode;
}

Client& Client::operator=(const Client& other) {
    this->player.color = other.player.color;
    this->clock = other.clock;
    this->player.name = other.player.name;
    this->player.radius = other.player.radius;
    this->player.mode = other.player.mode;

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
    sf::Time timestamp = now - sf::milliseconds(network.ping);

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

void Client::compensationInterpolation() {
    std::unordered_map<std::string, State> pastState = bufferOnReceipt.getTState(-1);
    std::unordered_map<std::string, State> currState = bufferOnReceipt.getCurrentState();
    for (auto & [name, other] : opponents) {
        if (name != getName()) {
            Position pastPos = pastState[name].getPosition();
            Position currPos = currState[name].getPosition();

            // Position = old one + diff. * (0 at beginning of tick, 1 at end of tick)
            double tickProgress = (clock.getElapsedTime().asMilliseconds() - lastServerTick) / (double)Const::TICKRATE.count();
            Position pos;

            pos.setX(pastPos.getX() + (currPos.getX() - pastPos.getX()) * tickProgress);
            pos.setY(pastPos.getY() + (currPos.getY() - pastPos.getY()) * tickProgress);

            opponents[name].position = pos;

            // If the radius goes through 0, make sure we rotate correctly.
            opponents[name].radius = pastState[name].getRadius() + (currState[name].getRadius() - pastState[name].getRadius()) * tickProgress;
        }
    }
}

void Client::compensationPrediction(Input inputs) {
    int now = clock.getElapsedTime().asMilliseconds();

    Position pos(getPlayer().position.getX(), getPlayer().position.getY());
    pos.move(inputs.getMovementX(), inputs.getMovementY(), now-lastUpdate);
    setPosition(pos);

    setRadius(getPlayer().radius + inputs.getRotate() * Const::PLAYER_RADIUS_SPEED * (now - lastUpdate));

    State state(now, pos, getRadius(), getPlayer().mode, getPlayer().isAttacking, getPlayer().wpn.getId(), inputs);
    inputsBuffer[lastInputId] = state;
}

void Client::compensationReconciliation() {
    State currentState = bufferOnReceipt.getCurrentState()[getName()];
    unsigned int lastReceivedInputs = currentState.getLastInputsId();

    if (inputsBuffer.begin()->first < lastReceivedInputs) {
        // If 1st element of buffer < last state received by server. (AKA if need to check for reconciliation)
        Position p = inputsBuffer[lastReceivedInputs].getPosition();
        Position q = currentState.getPosition();
        ImVec2 diff = {p.getX() - q.getX(), p.getY() - q.getY()};
        if (sqrt(pow(diff.x, 2) + pow(diff.y, 2)) > Const::PLAYER_SPEED * 20) { // If diff. of pos > eq. of 20ms of movement:
            Position pos = getPosition();
            pos.setX(pos.getX() - diff.x/2);
            pos.setY(pos.getY() - diff.y/2);
            setPosition(pos);
        }

        if (std::fmod(inputsBuffer[lastReceivedInputs].getRadius() - currentState.getRadius(), 2*std::numbers::pi) > 2*std::numbers::pi/180) { // If radius diff. > 2°
            setRadius(currentState.getRadius());
        }

        // Delete all frames up to this one (as it has already been processed)
        auto it = inputsBuffer.begin();
        while (it != inputsBuffer.end() and it->first <= lastReceivedInputs) {
            ++it;
        }
        inputsBuffer.erase(inputsBuffer.begin(), it);
    }
}
