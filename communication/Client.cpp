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
Client::Client(sf::Clock& clock, Console& console, std::string name, short controller, sf::Color color) :
    server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4), semaphore(1),
    player(std::move(name), color, 0),
    console(console),
    clock(clock) {
    this->bufferOnReceipt.addClient(player);
    this->controllerNumber = controller;
    this->updateThread = std::thread(&Client::update, this);
}

Client::~Client() {
    running = false;
    socket.unbind();
    if (updateThread.joinable()) {
        updateThread.join();
    }
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
        this->player.setPort(0);
    }
    else {
        player.setPort(socket.getLocalPort());
        std::cout << "Client " << getName() << " started on port: " << player.getPort() << std::endl;

        socket.setBlocking(true);

        sendThread = std::thread(&Client::sendLoop, this);
        receiveThread = std::thread(&Client::receiveLoop, this);
    }

    std::unordered_map<std::string, std::any> infos = {
            {"error", player.getPort() == 0},
            {"name", getName()},
            {"port", player.getPort()},
            {"color", player.getColor()}
    };

    return infos;
}

Player Client::getPlayer() {
    return player;
}

std::string Client::getName() {
    return player.getName();
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
    return this->player.getRadius();
}

Position Client::getPosition() const {
    return this->player.getPosition();
}

int Client::getStatus() const {
    return this->player.getStatus();
}

bool Client::getLoop() const {
    return this->loop;
}

short Client::getMapID() const {
    return this->mapID;
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

int Client::getTick() const {
    return clock.getElapsedTime().asMilliseconds();
}

void Client::setStatus(int status) {
    this->player.setStatus(status);
}

void Client::setMapID(short mapID) {
    this->mapID = mapID;
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

void Client::setPlayer(Player player) {
    this->player = player;
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
    // TODO: Fix SIGSEGV error when closing game caused by this (??)
    Input input;
    float value = 0;
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
                if(this->player.getTimer_atk() == -1) {
                    input.handleInput(i.first, value);
                    break;
                }
            case Inputs::WPN_CCW :
                if(this->player.getTimer_atk() == -1) {
                    input.handleInput(i.first, value);
                    break;
                }
            case Inputs::WPN_CHANGE:
                if(value > 0.f && !mode_enable && this->player.getTimer_atk() == -1) {
                    input.setChangeWpn(true);
                }
                input.setModeEnable(value > 0.f);
                break;

            case Inputs::ATTACK:
                if(value > 0.f && !attack_enable && this->player.getTimer_atk() == -1) {
                    input.setAttack(true);
                }
                input.setAttackEnable(value > 0.f);
                break;

            case Inputs::WPN_ANGLE_NS:
                input.setOnController(true);
                weaponAngle.x = value*100;
                break;

            case Inputs::WPN_ANGLE_WE:
                input.setOnController(true);
                weaponAngle.y = value*100;
                break;

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
void Client::update() {
    bool mode_enable = true;    // set the ability to change the weapon to true at the beginning
    bool attack_enable = true;  // set the ability to attack at true at the beginning

    int before = 0;

    while (running) {
        while (!loop) {sf::sleep(sf::Time());} // Pause if needed
        // ==========| INPUTS |========== //
        Input input = this->getInputs(mode_enable, attack_enable);
        int t = clock.getElapsedTime().asMilliseconds();
        mode_enable = input.getModeEnable();
        attack_enable = input.getAttackEnable();

        // Storing recent local positions to re-adjust if needed.
        if (player.getStatus() == Status::DONE) {
            // float radius = getRadius();
            if (input.getRotate() == -999) { // If user on controller and not moving stick:
                input.setRotate(player.getRadius()); // Get last radius pos.
            }

            // We change input ID if inputs held different than last ones, or if held for more than 100ms
            input.setId(lastInputId);
            m_inputs.lock();
            if (!inputs.empty()) {
                auto it = inputs.lower_bound(lastSentTick); // Get last input < last tick sent to server
                if (it != inputs.begin()) { // If inputs array is not empty:
                    auto lastBelow = std::prev(it)->second;
                    inputs.erase(inputs.begin(), it); // We erase all inputs before last tick sent

                    // If the last input erased isn't empty:
                    if (lastBelow != Input()) {
                        lastBelow.setId(lastInputId++); // We increment its ID for next packet
                        lastBelow.setChangeWpn(false);
                        lastBelow.setAttack(false);
                    }
                    inputs[lastSentTick] = lastBelow; // We re-add last input in array:
                }

                // If input is different than last one, or is held for more than 50ms AND not empty:
                if (input != std::prev(inputs.end())->second or (t - std::prev(inputs.end())->first > 500/tickrate and input != Input())) {
                    // We do +1 to ID and add it to inputs array.
                    input.setId(lastInputId++);
                    inputs.insert({t, input});
                }
            }
            else if (input != Input()) { // If input isn't empty and user holding a key:
                input.setId(lastInputId++);
                inputs.insert({t, input});
            }
        }
        else {
            lastInputId = 0;
            inputsBuffer.clear();
        }

        // ==========| COMPENSATIONS (if needed) |========== //
        semaphore.acquire();
        State state(t, getPosition(), input, getRadius(), getPlayer().getIsAttacking(), getPlayer().getWpn().getId());

        if (getCompensationEnabled(Compensation::INTERPOLATION)) {
            compensationInterpolation();
        }
        if (getCompensationEnabled(Compensation::PREDICTION)) {
            compensationPrediction(input, t);
        }
        if (getCompensationEnabled(Compensation::RECONCILIATION)) {
            compensationReconciliation();
        }
        else { // If not reconciliation, we empty inputsBuffer to avoid SIGSEGV/huge memory alloc.:
            inputsBuffer.clear();
        }

        inputsBuffer[input.getId()] = state; // We re-add the last input post-deletion, as it may be used for controller players. (R-stick)
        m_inputs.unlock();

        lastUpdate = t;
        semaphore.release();

        // ===== ATTACK =====
        player.handleTimer_atk(lastUpdate, before);
        for(auto& [name, opp] : opponents) {
            opp.handleTimer_atk(lastUpdate, before);
        }
        before = lastUpdate;
        sf::sleep(sf::milliseconds(1000/Const::DISPLAY_REFRESH_RATE)); // Small sleep to make sure everyone send same amount of inputs.
    }
}


/**
 * Loop that executes every tick rate: client will calculate if packet must be dropped or not, and sends packet of
 * position to the server. This function shouldn't return, except if the server stops and the client should stop
 * executing.
 */
void Client::sendLoop() {
    while (!loop) {sf::sleep(sf::Time());} // Pause if needed
    while (running) {
        QueuedPacket pkt;
        short type = 0;
        semaphore.acquire();
        pkt.timestamp = sf::milliseconds(lastUpdate);
        lastSentTick = lastUpdate;
        uint32_t id = getPacketId();
        pkt.packet << id;
        pkt.packetID = id;
        semaphore.release();

        switch (player.getStatus()) {
            // Player sends their data to the server, waiting for ACK:
            case Status::WAITING_FOR_INIT: {
                type = Pkt::NEW_PLAYER;
                pkt.packet << Pkt::NEW_PLAYER;
                pkt.packet << player.getName() << player.getColor().r << player.getColor().g << player.getColor().b << player.getColor().a;
                pkt.packet << player.getWeapons()[1]; // Add current weapon as well.

                break;
            }

            // Player is waiting for data of their opponents:
            case Status::WAITING_FOR_OPPONENTS: {
                type = Pkt::ACK;
                // Tells the server it ACKd that it has been added to the players pool.
                pkt.packet << Pkt::ACK << Pkt::NEW_PLAYER;
                break;
            }

            // Player is ready to start and gives the info to the server:
            case Status::READY_TO_START: {
                type = Pkt::ACK;
                pkt.packet << Pkt::ACK << Pkt::READY_R << int(opponents.size());
                break;
            }
            // Game has been started:
            case Status::DONE: {
                Input lastInput;
                type = Pkt::INPUTS;
                pkt.packet << Pkt::INPUTS << int(pkt.timestamp.asMilliseconds());
                m_inputs.lock();
                for (auto & [dt, input] : inputs) {
                    pkt.packet << int(dt) << input;
                }
                m_inputs.unlock();

                break;
            }
            case Status::DEAD: {
                type = Pkt::END_R;
                pkt.packet << Pkt::END_R;
                break;
            }
            case Status::WIN: {
                type = Pkt::END_R;
                pkt.packet << Pkt::END_R;
                break;
            }
            // Unrecognized player status.
            default: {
                std::cout << "Unhandled player status: status #" << player.getStatus() << std::endl;
            }
        }

        semaphore.acquire();
        console.addPacket(pkt.packetID, type, player.getPort(), pkt.timestamp.asMilliseconds());
        queuedPackets[SENT].push_back(pkt); // Adds the packet to the array of packets.
        auto packetToSend = getLatestQueuedPacket(SENT);

        int packetLossChance = std::experimental::randint(1, 100);
        // Only executes if packet loss % is respected (randomly generated number)
        if (packetLossChance > network.packetLoss[1]) {
            if (packetToSend.has_value()) {
                socket.send(packetToSend.value().packet, server, COMM_PORT_SERVER);
            }
        }

        semaphore.release();

        sf::sleep(sf::milliseconds(1000/clientRefreshRate));
    }
}

void Client::receiveLoop() {
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve(SERVER_IP);
    sf::Packet rawPacket;
    sf::Packet packet;
    short type;
    short typeAck;
    unsigned short port;
    uint32_t id;

    while (running) {
        while (!loop) {sf::sleep(sf::Time());} // Pause if needed
        rawPacket.clear();
        sf::sleep(sf::Time()); // "empty" sleep: required for loops.
        if (socket.receive(rawPacket, sender, port) == sf::Socket::Status::Done) {
            int packetLossChance = std::experimental::randint(1, 100);
            // Only executes if packet loss % is respected (randomly generated number)
            if (packetLossChance > network.packetLoss[0]) {
                if (port == COMM_PORT_SERVER) {
                    QueuedPacket pkt;
                    pkt.packet = rawPacket;
                    pkt.timestamp = clock.getElapsedTime();
                    queuedPackets[RECEIVED].push_back(pkt);

                    auto qpacket = getLatestQueuedPacket(RECEIVED);

                    if (qpacket.has_value()) {
                        packet = qpacket->packet;
                        packet >> id >> type;
                        switch (type) {
                            case Pkt::ACK: {
                                packet >> typeAck;
                                switch (typeAck) {
                                    // Server ACKd player:
                                    case Pkt::NEW_PLAYER: {
                                        player.setStatus(Status::WAITING_FOR_OPPONENTS);
                                        break;
                                    }

                                    // All users are added, server is waiting for everyone to receive all data:
                                    case Pkt::WAIT_START_R: {
                                        player.setStatus(Status::READY_TO_START);
                                        break;
                                    }

                                    case Pkt::WAIT_OPPONENTS: {
                                        std::cout << "Not everyone is ready!" << std::endl;
                                        player.setStatus(Status::WAITING_FOR_OPPONENTS);
                                        break;
                                    }

                                    default: {
                                        std::cout
                                                << "Unrecognized acknowledge packet received in client section, please identify yourself "
                                                << typeAck << std::endl;
                                        break;
                                    }
                                }
                                break;
                            }

                            // Server is ready and sends the data and position of each client:
                            case Pkt::READY_R: {
                                // getting the map
                                int mapID;
                                packet >> mapID;
                                setMapID(mapID);

                                std::string name;
                                std::uint8_t r, g, b, a;
                                short weapon;

                                // Loops while we have more opponents to unpack:
                                while (packet >> name >> r >> g >> b >> a >> weapon) {
                                    // Drop if represents local instance:
                                    if (name != getName()) {
                                        // Otherwise, add the new player in opponents list:
                                        opponents[name] = Player(name, sf::Color(r, g, b, a));
                                        opponents[name].setWeapons({Weapons::SHIELD, weapon});

                                        std::cout << "Adding opponent: " << name << std::endl;
                                    }
                                }

                                player.setStatus(Status::READY_TO_START);

                                break;
                            }

                            case Pkt::GLOBAL: {
                                player.setStatus(Status::DONE);
                                int tick;
                                std::string name;
                                State state;

                                packet >> tick >> lastServerTick;

                                while (packet >> name >> state) {
                                    m_states.lock();
                                    std::unordered_map<std::string, State> currentState = bufferOnReceipt.getCurrentState();
                                    std::unordered_map<std::string, State> pastState = bufferOnReceipt.getTState(-1);
                                    m_states.unlock();

                                    if (name == this->getName()) {
                                        this->bufferOnReceipt.setNextPlayerState(player, state);
                                        m_states.lock();
                                        State currState = bufferOnReceipt.getLastState(player);
                                        m_states.unlock();

                                        if (!this->getCompensations()[Compensation::RECONCILIATION]) {
                                            this->player.setPosition(currState.getPosition());
                                            this->player.setRadius(currState.getRadius());
                                        }
                                        this->player.setIsAttacking(state.getAttack());
                                        this->player.setWpn(state.getWpn().getId());
                                        this->player.setPoint(state.getPoint());
                                    }
                                    else {
                                        // Opponent position:
                                        this->bufferOnReceipt.setNextPlayerState(opponents[name], state);
//                                        if (!this->getCompensationEnabled(Compensation::INTERPOLATION)) {
                                        opponents[name].setPosition(currentState[name].getPosition());
                                        opponents[name].setRadius(currentState[name].getRadius());
//                                        }
                                        opponents[name].setWpn(currentState[name].getWpn().getId());
                                        opponents[name].setIsAttacking(currentState[name].getAttack());
                                        opponents[name].setPoint(currentState[name].getPoint());
                                    }

                                    state.flushInputs();
                                }
                                m_states.lock();
                                this->bufferOnReceipt.push(tick);
                                m_states.unlock();
                                break;
                            }
                            case Pkt::DEATH: {     // tick << killerName                                                           // send the signal to a specific player that the player is dead
                                this->player.setPoint(0);
                                if (player.getStatus() == Status::DONE) {
                                    this->player.setStatus(Status::DEAD);
                                }
                                break;
                            }
                            case Pkt::WIN: {
                                if (player.getStatus() == Status::DONE) {
                                    player.setStatus(Status::WIN);
                                }
                                break;
                            }
                            case Pkt::END_R: {     // tick                                                                         // send the signal that the round is finished
                                std::cout << "receive end R packet" << std::endl;
                                // TODO : Deleting the actual player and opponent, putting the packetTypeToSend on "None" to wait for the new player
                                break;
                            }
                            default: {
                                std::cout << "UNKNOWN PACKET RECEIVE CLIENT! Type: " << type << std::endl;
                                break;
                            }
                        }
                        semaphore.acquire();
                        console.addPacket(id, type, this->player.getPort(), clock.getElapsedTime().asMilliseconds(), true);
                        semaphore.release();
                    }
                }
            }
        }
    }
}

sf::Color Client::getColor() {
    return this->player.getColor();
}

// Copy constructors
Client::Client(const Client& other) : server(other.server), semaphore(1),
    player(other.player.getName(), other.player.getColor()),
    console(other.console) {
    this->player.setPosition(other.player.getPosition());
    this->player.setRadius(other.player.getRadius());
    this->player.setWpn(other.player.getWpn().getId());
    this->player.setWeapons(other.player.getWeapons());
}

Client& Client::operator=(const Client& other) {
    if (this != &other) {
        this->player.setName(other.player.getName());
        this->player.setColor(other.player.getColor());
        this->player.setPosition(other.player.getPosition());
        this->player.setRadius(other.player.getRadius());
        this->player.setWpn(other.player.getWpn().getId());
        this->player.setWeapons(other.player.getWeapons());
    }

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
std::optional<QueuedPacket> Client::getLatestQueuedPacket(int status) {
    // Shouldn't happen: only if queuedPackets is empty.
    if (queuedPackets[status].empty()) {
        return std::nullopt;
    }

    sf::Time now = clock.getElapsedTime();
    sf::Time timestamp = now - sf::milliseconds(network.ping[status]);

    QueuedPacket toSend;
    bool found = false;

    // Check in the queuedPackets until we find the right one.
    while (!queuedPackets[status].empty() && queuedPackets[status].front().timestamp <= timestamp) {
        toSend = queuedPackets[status].front();
        queuedPackets[status].pop_front();
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
    m_states.lock();
    std::unordered_map<std::string, State> pastState = bufferOnReceipt.getTState(-1);
    std::unordered_map<std::string, State> currState = bufferOnReceipt.getCurrentState();
    m_states.unlock();
    for (auto & [name, other] : opponents) {
        if (name != getName()) {
            Position pos = opponents[name].getPosition();
            float radius = opponents[name].getRadius();

            double tickProgress = std::min(1.0,(clock.getElapsedTime().asMilliseconds() - lastServerTick) / (1000/(double)tickrate));
            Input corrInputs = currState[name].getPercentInput(tickProgress);
            pos.move(corrInputs.getMovementX(), corrInputs.getMovementY(), clock.getElapsedTime().asMilliseconds() - lastUpdate);

            opponents[name].setPosition(pos);

            // If the radius goes through 0, make sure we rotate correctly.
            float diff = currState[name].getRadius() - pastState[name].getRadius();
            if (corrInputs.getOnController()) {
                radius += diff * tickProgress;
            }
            else {
                radius += Const::PLAYER_RADIUS_SPEED * corrInputs.getRotate() * (clock.getElapsedTime().asMilliseconds() - lastUpdate);
            }
            opponents[name].setRadius(radius);
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
 * @param inputs Inputs played by the player at that corresponding frame.
 *
 * @attention Depending of ping, a "rollback" might appear when enabling Prediction without Reconciliation. This is
 * expected, and caused by the server still having the "last word" regarding the client's position.
 */
void Client::compensationPrediction(Input inputs, int now) {
    Position pos = getPlayer().getPosition();
    pos.move(inputs.getMovementX(), inputs.getMovementY(), now-lastUpdate);
    player.setPosition(pos);

    if (inputs.getOnController()) {
        player.setRadius(inputs.getRotate());
    }
    else {
        player.setRadius(getPlayer().getRadius() + inputs.getRotate() * Const::PLAYER_RADIUS_SPEED * (now - lastUpdate));
    }

    State state(now, player.getPosition(), inputs, player.getRadius(), player.getIsAttacking(), player.getWpn().getId(), player.getPoint());

    inputsBuffer[lastInputId] = state;
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
    m_states.lock();
    State currentState = bufferOnReceipt.getCurrentState()[getName()];
    m_states.unlock();
    unsigned int lastReceivedInputs = currentState.getLastInputsId();

    if (inputsBuffer.begin()->first <= lastReceivedInputs and inputsBuffer.contains(lastReceivedInputs)) {
        // If 1st element of buffer < last state received by server. (AKA if need to check for reconciliation)
        auto lastLocalInputs = inputsBuffer[lastReceivedInputs];
        Position p = lastLocalInputs.getPosition(); // Local pos.
        Position q = currentState.getPosition(); // Server pos. [received]
        // We roll-back to be at same timestamp than server:
        int dt = currentState.getTimestamp() - lastLocalInputs.getTimestamp();
        p.move(lastLocalInputs.getInputs().begin()->second.getMovementX(), lastLocalInputs.getInputs().begin()->second.getMovementY(), dt);
       ImVec2 diff = {p.getX() - q.getX(), p.getY() - q.getY()};

        if (sqrt(pow(diff.x, 2) + pow(diff.y, 2)) >= Const::PLAYER_SPEED * 100 * 5) { // If diff. of pos > eq. of x ms of movement:
            std::cout << "Diff of: " << diff.x << "; " << diff.y << std::endl;
            Position pos = getPosition();
            pos.setX(pos.getX() - diff.x);
            pos.setY(pos.getY() - diff.y);

            // Re-emulates all past positions:
            for (auto& [tick, state] : inputsBuffer) {
                if (tick > lastReceivedInputs) {
                    pos = state.getPosition();
                    pos.setX(pos.getX() - diff.x);
                    pos.setY(pos.getY() - diff.y);
                    inputsBuffer[tick].setPosition(pos);
                }
            }
            player.setPosition(pos);
        }

        if (std::fmod(inputsBuffer[lastReceivedInputs].getRadius() - currentState.getRadius(), 2*std::numbers::pi) > 1*std::numbers::pi/180) { // If radius diff. > 1°
            player.setRadius(currentState.getRadius());
        }

        // Delete all frames up to this one (as it has already been processed)
        auto it = inputsBuffer.begin();
        while (it != inputsBuffer.end() and it->first < lastReceivedInputs) {
            ++it;
        }
        inputsBuffer.erase(inputsBuffer.begin(), ++it);
    }
}

void Client::setLoop(bool loop) {
    this->loop = loop;

    // If we stopped our last game, empty opponents:
    if (loop) {
        opponents.clear();
        lastInputId = 0;
        inputsBuffer.clear();
    }
}