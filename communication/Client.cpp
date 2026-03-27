#include "Client.hpp"

#include <algorithm>
#include <utility>
#include <bits/ranges_algo.h>
#include <SFML/Window/Keyboard.hpp>

/**
 * Clients are being created by ClientUI (which extends this class). A client will have all the information needed
 * for the server-client synchronisation, such as packet loss, ping, name (identification), ...
 *
 * @param clock Clock to synchronise with the server. Needed to sync queuedPackets!
 * @param name Name given to the client. Can be any string, must be unique!
 * @param color Color given to the client in the Server's console.
 */
Client::Client(const sf::Clock clock, std::string name, short controller, sf::Color color) : server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4), semaphore(1) {
    this->clock = clock;
    this->player.name = std::move(name);
    this->player.color = color;
    this->bufferOnReceipt.addClient(player);
    this->player.radius = std::numbers::pi/2; //put the element on top in radius
    this->player.weapons = {1, 0};
    this->player.isAttacking = false;
    this->player.timer_atk = -1;
    this->controllerNumber = controller;

    this->player.status = Status::WAITING_FOR_INIT;

    Weapon wpn (player.weapons[0]); // ID of the default wpn
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

int Client::getReceivingPacketLoss() const {
    return network.packetLoss[0];
}

int Client::getSendingPacketLoss() const {
    return network.packetLoss[1];
}

int Client::getReceivingPing() const {
    return network.ping[0];
}

int Client::getSendingPing() const {
    return network.ping[1];
}

float Client::getRadius() const {
    return this->player.radius;
}

Position Client::getPosition() const {
    return this->player.position;
}

int Client::getStatus() const {
    return this->player.status;
}

bool Client::getLoop() const {
    return this->loop;
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

void Client::setStatus(int status) {
    this->player.status = status;
}

void Client::setLoop(bool loop) {
    this->loop = loop;
}

void Client::setRadius(float radius) {
    this->player.radius = std::fmod(radius, 2.f * std::numbers::pi);
}

void Client::setReceivingPacketLoss(int packetLoss) {
    if (packetLoss >= 0 and packetLoss <= 100) {
        this->network.packetLoss[0] = packetLoss;
    }
}

void Client::setSendingPacketLoss(int packetLoss) {
    if (packetLoss >= 0 and packetLoss <= 100) {
        this->network.packetLoss[1] = packetLoss;
    }
}

void Client::setReceivingPing(int ping) {
    if (ping >= 0) {
        this->network.ping[0] = ping;
    }
}

void Client::setSendingPing(int ping) {
    if (ping >= 0) {
        this->network.ping[1] = ping;
    }
}

void Client::setCompensations(std::array<bool,3> compensations) {
    this->network.compensations = compensations;
}

void Client::setKeybinds(std::unordered_map<int, std::variant<sf::Keyboard::Key, sf::Joystick::Axis, int>> keybinds) {
    this->keybinds = std::move(keybinds);
}

void Client::setController(short controller) {
    this->controllerNumber = controller;
}


/**
 * Function that recovers all the users inputs. This uses user's defined keybinds and iterates
 * over each of them to construct an object called "Input". This function calls a function named
 * "handleInput", from the object "Input", which will put the correct key/value into the correct
 * variable in the object Input.
 *
 * @param mode_enable (<optional> false by default) - Allows you to give the function last weapon changeWpn value (attack/defense)
 * @param attack_enable (<optional> true by default) - Allows the user to attack (should be false if cooldown, switching weapon, ...)
 * @return - Returns an object Input with corresponding values according to keys pressed.
 */
Input Client::getInputs(bool mode_enable, bool attack_enable) {
    Input input;
    float value;
    ImVec2 weaponAngle = {};

    for (const std::pair<const int, std::variant<sf::Keyboard::Key, sf::Joystick::Axis, int>> & i : keybinds) {
        // If keybind is on keyboard:
        if (std::holds_alternative<sf::Keyboard::Key>(i.second)) {
            value = isKeyPressed(std::get<sf::Keyboard::Key>(i.second));
        }
        // Else, if it's a joystick:
        else if (std::holds_alternative<sf::Joystick::Axis>(i.second)) {
            value = sf::Joystick::getAxisPosition(controllerNumber, std::get<sf::Joystick::Axis>(i.second)) / 100; // Axis values are between -100 and 100.
        }
        // Else, if it's a controller button:
        else if (std::holds_alternative<int>(i.second)) {
            value = sf::Joystick::isButtonPressed(controllerNumber, std::get<int>(i.second));
        }

        switch (i.first) {
            case Inputs::WPN_CW :
                if(this->player.timer_atk == -1) {
                    input.handleInput(i.first, value);
                    break;
                }
            case Inputs::WPN_CCW :
                if(this->player.timer_atk == -1) {
                    input.handleInput(i.first, value);
                    break;
                }
            case Inputs::WPN_CHANGE:
                if(value > 0.f && !mode_enable && this->player.timer_atk == -1) {
                    input.setChangeWpn(true);
                }

                input.setModeEnable(value > 0.f);
                break;
            case Inputs::ATTACK:
                if(value > 0.f && !attack_enable && this->player.timer_atk == -1) {
                    input.setAttack(true);
                }
                input.setAttackEnable(value > 0.f);
                break;
            case Inputs::WPN_ANGLE_NS:
                input.setOnController(true);
                weaponAngle.x = value;

            case Inputs::WPN_ANGLE_WE:
                input.setOnController(true);
                weaponAngle.y = value;

            default:
                input.handleInput(i.first, value);
                break;
        }
    }

    // Check if weapon angle is adjusted through controller:
    if (input.getOnController()) {
        if (fabs(weaponAngle.x) < Const::CONTROLLER_DEADZONE and fabs(weaponAngle.y) < Const::CONTROLLER_DEADZONE) {
            input.setRotate(-999);
        }
        else {
            float angle = atan2(weaponAngle.y, -weaponAngle.x);
            angle -= M_PI/2; // Adjust for in-game angle.
            if (angle < 0) {
                angle += 2*M_PI;
            }
            input.setRotate(angle);
        }
    }

    return input;
}

/**
 * @brief Loops that executes at every frame. Gather inputs, send packet and applies compensations.
 * @return - Error code. (Const. Err:: in Utils.hpp)
 */
int Client::update() {
    bool mode_enable = true;    // set the ability to change the weapon to true at the beginning
    bool attack_enable = true;  // set the ability to attack at true at the beginning

    int before = 0;

    while (loop) {
        // ==========| INPUTS |========== //
        Input inputs = this->getInputs(mode_enable, attack_enable);
        mode_enable = inputs.getModeEnable();
        attack_enable = inputs.getAttackEnable();

        // Storing recent local positions to re-adjust if needed.
        if (player.status == Status::DONE) {
            float radius = getRadius();
            if (inputs.getRotate() == -999) { // If user on controller and not moving stick:
                inputs.setRotate(player.radius); // Get last radius pos.
            }
            State state(clock.getElapsedTime().asMilliseconds(), getPlayer().position, inputs, radius, getPlayer().isAttacking, getPlayer().wpn.getId());
            inputs.setId(lastInputId);
            inputsBuffer[lastInputId] = state;
            lastInputId++;
        }
        else {
            lastInputId = 0;
            inputsBuffer.clear();
        }

        // ==========| PACKET HANDLER |========== //
        // Verifying if we should drop packet (packet loss %):
        int packetLossChance = std::experimental::randint(1, 100);
        if (packetLossChance > network.packetLoss[1]) {
            // Add latest inputs to queue (for ping simulation)
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
        else { // If not reconciliation, we empty inputsBuffer to avoid SIGSEGV/huge memory alloc.:
            inputsBuffer.clear();
        }
        State state(lastUpdate, getPosition(), inputs, getRadius(), getPlayer().isAttacking, getPlayer().wpn.getId());
        inputsBuffer[lastInputId] = state; // We add the last input as it may be used for controller players. (R-stick)
        semaphore.release();

        lastUpdate = clock.getElapsedTime().asMilliseconds();

        // ===== ATTACK =====
        if(player.isAttacking && player.timer_atk == -1) {
            player.timer_atk = lastUpdate - before;
        }
        if(player.timer_atk != -1) {
            player.timer_atk += lastUpdate - before;
        }
        if(player.timer_atk >= player.wpn.getAttackSpeed() + player.wpn.getReload()) {
            player.timer_atk = -1;
        }

        for(auto& [name, opp] : opponents) {
            if(opp.isAttacking && opp.timer_atk == -1) {
                opp.timer_atk = lastUpdate - before;
            }
            if(opp.timer_atk != -1) {
                opp.timer_atk += lastUpdate - before;
            }
            if(opp.timer_atk >= opp.wpn.getAttackSpeed() + opp.wpn.getReload()) {
                opp.timer_atk = -1;
            }
        }
        before = lastUpdate;
        sf::sleep(sf::milliseconds(5)); // Small sleep to make sure everyone send same amount of inputs.
    }

    return Err::ERR_NONE;
}

/**
 * Loop that executes every tick rate: client will calculate if packet must be dropped or not, and sends packet of
 * position to the server. This function shouldn't return, except if the server stops and the client should stop
 * executing.
 */
int Client::sendPacket(Input inputs) {
    QueuedPacket pkt;
    pkt.timestamp = clock.getElapsedTime();

    switch (player.status) {
        // Player sends their data to the server, waiting for ACK:
        case Status::WAITING_FOR_INIT: {
            pkt.packet << Pkt::NEW_PLAYER;
            pkt.packet << player.name << player.color.r << player.color.g << player.color.b << player.color.a;
            pkt.packet << player.wpn.getId();
            break;
        }

        // Player is waiting for data of their opponents:
        case Status::WAITING_FOR_OPPONENTS: {
            // Tells the server it ACKd that it has been added to the players pool.
            pkt.packet << Pkt::ACK << Pkt::NEW_PLAYER;
            break;
        }

        // Player is ready to start and gives the info to the server:
        case Status::READY_TO_START: {
            pkt.packet << Pkt::ACK << Pkt::READY_R << int(opponents.size());
            break;
        }
        // Game has been started:
        case Status::DONE: {
            pkt.packet << Pkt::INPUTS << int(pkt.timestamp.asMilliseconds()) << inputs << player.port;
            break;
        }
        case Status::DEAD: {
            pkt.packet << Pkt::END_R;
            break;
        }
        // Unrecognized player status.
        default: {
            std::cout << "Unhandled player status: status #" << player.status << std::endl;
            return Err::PLAYER_STATUS_UNSYNCED;
        }
    }
    queuedPackets.push_back(pkt); // Adds the packet to the array of packets.
    auto packetToSend = getLatestQueuedPacket();
    if (packetToSend.has_value()) {
        socket.send(packetToSend.value(), server, COMM_PORT_SERVER);
    }

    return Err::ERR_NONE;
}

int Client::receiveLoop() {
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve(SERVER_IP);
    sf::Packet packet;
    int type;
    int typeAck;
    const sf::Time tickrate = std::chrono::milliseconds(TICKRATE);
    short unsigned int port;

    while (loop) {
        packet.clear();
        sf::sleep(sf::Time()); // "empty" sleep: required for loops.
        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {
            int packetLossChance = std::experimental::randint(1, 100);
            // Only executes if packet loss % is respected (randomly generated number)
            if (packetLossChance > network.packetLoss[0]) {
                if (port == COMM_PORT_SERVER) {
                    packet >> type;
                    switch (type) {
                        case Pkt::ACK: {
                            packet >> typeAck;
                            switch(typeAck) {
                                // Server ACKd player:
                                case Pkt::NEW_PLAYER: {
                                    player.status = Status::WAITING_FOR_OPPONENTS;
                                    break;
                                }

                                // All users are added, server is waiting for everyone to receive all data:
                                case Pkt::WAIT_START_R: {
                                    player.status = Status::READY_TO_START;
                                    break;
                                }

                                case Pkt::WAIT_OPPONENTS: {
                                    std::cout << "Not everyone is ready!" << std::endl;
                                    player.status = Status::WAITING_FOR_OPPONENTS;
                                    break;
                                }

                                default: {
                                    std::cout << "Unrecognized acknowledge packet received in client section, please identify yourself " << typeAck << std::endl;
                                    break;
                                }
                            }
                            break;
                        }

                        // Server is ready and sends the data and position of each client:
                        case Pkt::READY_R: {
                            std::string name;
                            std::uint8_t r, g, b, a;

                            // Loops while we have more opponents to unpack:
                            while (packet >> name >> r >> g >> b >> a) {
                                // Drop if represents local instance:
                                if (name != getName()) {
                                    // Otherwise, add the new player in opponents list:
                                    Player pl;
                                    pl.name = name;
                                    pl.color = sf::Color(r, g, b, a);
                                    opponents[name] = pl;

                                    std::cout << "Adding opponent: " << name << std::endl;
                                }
                            }

                            player.status = Status::READY_TO_START;

                            break;
                        }

                        case Pkt::GLOBAL: {
                            player.status = Status::DONE;
                            int tick;
                            std::string name;
                            State state;

                            packet >> tick >> lastServerTick;

                            while (packet >> name >> state) {
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
                                        this->player.radius = currState.getRadius();
                                    }
                                    this->player.isAttacking = state.getAttack();
                                    this->player.isAttacking = state.getAttack();
                                    this->player.wpn.applyID(state.getWpn().getId());
                                    this->player.point = state.getPoint();
                                } else {
                                    // Opponent position:
                                    this->bufferOnReceipt.updateNextPlayerState(opponents[name], state);
                                    opponents[name].position.setX(currentState[name].getPosition().getX());
                                    opponents[name].position.setY(currentState[name].getPosition().getY());
                                    opponents[name].radius = currentState[name].getRadius();
                                    opponents[name].isAttacking = currentState[name].getAttack();
                                    opponents[name].wpn.applyID(currentState[name].getWpn().getId());
                                    opponents[name].point = currentState[name].getPoint();
                                }
                            }
                            this->bufferOnReceipt.push(tick);
                            break;
                        }
                        case Pkt::DEATH: {     // tick << killerName                                                           // send the signal to a specific player that the player is dead
                            this->player.point = 0;
                            this->player.status = Status::DEAD;
                            break;
                        }
                        case Pkt::END_R: {     // tick                                                                         // send the signal that the round is finished
                            std::cout<<"receive end R packet"<<std::endl;
                            // TODO : Deleting the actual player and opponent, putting the packetTypeToSend on "None" to wait for the new player
                            break;
                        }
                        default: {
                            std::cout << "UNKNOWN PACKET RECEIVE CLIENT! Type: " << type << std::endl;
                            break;
                        }
                    }
                }
            }
        }
    }

    sendThread.join();

    return Err::ERR_NONE;
}

void Client::applyState(std::string name, State state){
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
        opponents[name].isAttacking = currentState[name].getAttack();
        opponents[name].wpn.applyID(currentState[name].getWpn().getId());
        opponents[name].point = currentState[name].getPoint();
    }
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
    this->player.wpn = other.player.wpn;
    this->player.weapons = other.player.weapons;
}

Client& Client::operator=(const Client& other) {
    this->player.color = other.player.color;
    this->clock = other.clock;
    this->player.name = other.player.name;
    this->player.radius = other.player.radius;
    this->player.wpn = other.player.wpn;
    this->player.weapons = other.player.weapons;

    return *this;
}

/**
 * Function that returns the latest packet. This will go through the queuedPackets vector of the Client object, will check for
 * ping value, and will return the last packet respecting the given context. If no packet matches the criteria, an
 * empty packet will be returned.
 * Please note that this will alter the queuedPackets deque!
 *
 * @return Latest packet respecting ping value. Empty packet if none corresponds.
 */
std::optional<sf::Packet> Client::getLatestQueuedPacket() {
    // Shouldn't happen: only if queuedPackets is empty.
    if (queuedPackets.empty()) {
        return std::nullopt;
    }

    sf::Time now = clock.getElapsedTime();
    sf::Time timestamp = now - sf::milliseconds(network.ping[1]);

    sf::Packet toSend;
    bool found = false;

    // Check in the queuedPackets until we find the right one.
    while (!queuedPackets.empty() && queuedPackets.front().timestamp <= timestamp) {
        toSend = queuedPackets.front().packet;
        queuedPackets.pop_front();
        found = true;
    }

    // Return the corresponding packet
    if (found) {
        return toSend;
    }

    // Shouldn't happen: error case.
    return std::nullopt;
}

/**
 * Function that allows the interpolation of enemies position and rotation.
 *
 * @details The "Interpolation" is a compensation method that allows the client to display every single opponent's position
 * between 2 received ticks from the server. This allows the client to see the game at its local, monitor refresh rate,
 * instead of being limited at the tickrate of the server.
 *
 * @attention Without this compensation method, the enemy positions might not look "sharp". This is because modern monitors
 * have a refresh rate of 60Hz if not more. A server's tickrate is usually kept between 10 and 20.
 */
void Client::compensationInterpolation() {
    std::unordered_map<std::string, State> pastState = bufferOnReceipt.getTState(-1);
    std::unordered_map<std::string, State> currState = bufferOnReceipt.getCurrentState();
    for (auto & [name, other] : opponents) {
        if (name != getName()) {
            Position pastPos = pastState[name].getPosition();
            Position currPos = currState[name].getPosition();

            // Position = old one + diff. * (0 at beginning of tick, 1 at end of tick)
            server;
            double tickProgress = std::min(1.0,(clock.getElapsedTime().asMilliseconds() - lastServerTick) / (double)Const::TICKRATE.count());
            Position pos;

            pos.setX(pastPos.getX() + (currPos.getX() - pastPos.getX()) * tickProgress);
            pos.setY(pastPos.getY() + (currPos.getY() - pastPos.getY()) * tickProgress);

            opponents[name].position = pos;

            // If the radius goes through 0, make sure we rotate correctly.
            float diff = currState[name].getRadius() - pastState[name].getRadius();
            diff = std::fmod(diff + 3 * M_PI, 2 * M_PI) - M_PI;
            opponents[name].radius = std::fmod(pastState[name].getRadius() + diff * tickProgress, 2*M_PI);
        }
    }
}

/**
 * Function that allows the prediction of "future"'s local player's position and rotation.
 *
 * @details The "Prediction" is a compensation method that allows a client to predict its next position. For this, the
 * user runs its own "game simulation". According to the given inputs and the last position, it will move the character
 * to the newly established position without waiting for server confirmation.
 *
 * @param inputs - Inputs played by the player at that corresponding frame.
 *
 * @attention Depending of ping, a "rollback" might appear when enabling Prediction without Reconciliation. This is
 * expected, and caused by the server still having the "last word" regarding the client's position.
 */
void Client::compensationPrediction(Input inputs) {
    int now = clock.getElapsedTime().asMilliseconds();

    Position pos(getPlayer().position.getX(), getPlayer().position.getY());
    pos.move(inputs.getMovementX(), inputs.getMovementY(), now-lastUpdate);
    setPosition(pos);

    if (inputs.getOnController()) {
        setRadius(inputs.getRotate());
    }
    else {
        setRadius(getPlayer().radius + inputs.getRotate() * Const::PLAYER_RADIUS_SPEED * (now - lastUpdate));
    }
}

/**
 * @brief Function that changes the local player's position and rotation according to the reconciliation compensation method.
 *
 * @details The "Reconciliation" is a compensation method that allows a client to produce its actions locally as soon as
 * the corresponding keys are pressed. Each inputs packet have an ID, allowing the client to verify that the estimated
 * position corresponds with the one sent back by the server. If it does, it just deletes the past states from its memory.
 * If it doesn't, it will re-simulate the entire game state from that point up until the present, to then re-adjust to the
 * newly estimated position.
 *
 * @attention Please note that the Reconciliation compensation method cannot work without the Prediction method!
 */
void Client::compensationReconciliation() {
    State currentState = bufferOnReceipt.getCurrentState()[getName()];
    unsigned int lastReceivedInputs = currentState.getLastInputsId();


    if (inputsBuffer.begin()->first <= lastReceivedInputs) {
        // If 1st element of buffer < last state received by server. (AKA if need to check for reconciliation)
        Position p = inputsBuffer[lastReceivedInputs].getPosition();
        Position q = currentState.getPosition();
        ImVec2 diff = {p.getX() - q.getX(), p.getY() - q.getY()};
        if (sqrt(pow(diff.x, 2) + pow(diff.y, 2)) > Const::PLAYER_SPEED * 20) { // If diff. of pos > eq. of 20ms of movement:
            Position pos = getPosition();
            pos.setX(pos.getX() - diff.x);
            pos.setY(pos.getY() - diff.y);

            // Re-emulates all past positions:
            for (auto& [tick, state] : inputsBuffer) {
                if (tick > lastReceivedInputs) {
                    Position pos = state.getPosition();
                    pos.setX(pos.getX() - diff.x);
                    pos.setY(pos.getY() - diff.y);
                    inputsBuffer[tick].setPosition(pos);
                }
            }
            setPosition(pos);
        }

        if (std::fmod(inputsBuffer[lastReceivedInputs].getRadius() - currentState.getRadius(), 2*std::numbers::pi) > 1*std::numbers::pi/180) { // If radius diff. > 1°
            setRadius(currentState.getRadius());
        }

        // Delete all frames up to this one (as it has already been processed)
        auto it = inputsBuffer.begin();
        while (it != inputsBuffer.end() and it->first <= lastReceivedInputs) {
            ++it;
        }
        inputsBuffer.erase(inputsBuffer.begin(), ++it);
    }
}