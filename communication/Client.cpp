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
Client::Client(const sf::Clock clock, std::string name, short controller, sf::Color color) : server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4), semaphore(1) {
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
    this->controllerNumber = controller;

    Weapon wpn (0);                         // ID of the default wpn
    this->player.wpn = wpn;
}

Client::Client(const sf::Clock clock, std::string name, short controller, sf::Color color, int wpn_id) : server(SERVER_IP_BYTE1, SERVER_IP_BYTE2, SERVER_IP_BYTE3, SERVER_IP_BYTE4), semaphore(1) {
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
    this->controllerNumber = controller;

    Weapon wpn (wpn_id);                         // ID of the default wpn
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

void Client::setKeybinds(std::unordered_map<int, std::variant<sf::Keyboard::Key, sf::Joystick::Axis, int>> keybinds) {
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
    float value;

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
                    input.setMode(true);
                }

                input.setModeEnable(value > 0.f);
                break;
            case Inputs::ATTACK:
                if(value > 0.f && !attack_enable && this->player.timer_atk == -1 && this->player.mode == true) {
                    input.setAttack(true);
                }
                input.setAttackEnable(value > 0.f);
                break;
            default:
                input.handleInput(i.first, value);
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
    packetTypeToSend = Pkt::NEW_PLAYER;  // TODO : while the main is not creating the player directly in the server, un-comment this
    // packetTypeToSend = Pkt::WAIT_START_R;
    sendPacket(this->getInputs());

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
    // QueuedPacket pkt;
    // pkt.timestamp = clock.getElapsedTime();
    QueuedPacket pkt;
    pkt.timestamp = clock.getElapsedTime();
    int tick = clock.getElapsedTime().asMilliseconds();

    switch (packetTypeToSend) {
        case Pkt::ACK               : {     // Pkt << tick
            pkt.packet << Pkt::ACK;

            switch (ackToSend) {
                case Pkt::READY_R   : {
                    pkt.packet << Pkt::READY_R << tick;
                    break;
                }

                case Pkt::DEATH     : {
                    pkt.packet << Pkt::DEATH << tick;
                    break;
                }

                case Pkt::END_R     : {
                    pkt.packet << Pkt::END_R << tick;
                    break;
                }

                default             : {
                    std::cout << "Unrecognized acknoledge packet send in client section, please identify yourself " << ackToSend << std::endl;
                    break;
                }
            }
            break;
        }
        case Pkt::NEW_PLAYER        : {     // tick << client.name << client.color << client.wpn << client.port
            pkt.packet << Pkt::NEW_PLAYER;
            short r = player.color.r;
            short g = player.color.g;
            short b = player.color.b;
            short a = player.color.a;
            pkt.packet << tick << player.name << r << g << b << a << player.wpn.getId() << player.port;
            // std::cout << "new player request "<< player.name << std::endl;
            break;
        }
        case Pkt::WAIT_START_R      : {     // tick << client.port
            pkt.packet << Pkt::WAIT_START_R;
            pkt.packet << tick << player.port;
            // std::cout << "player " << player.name << " is waiting to start ! " << std::endl;
            break;
        }
        case Pkt::INPUTS            : {     // tick << inputs << client.port
            pkt.packet << Pkt::INPUTS << pkt.timestamp.asMilliseconds() << inputs << player.port;
            break;
        }
        case Pkt::END_GAME          : {     // tick << client.port
            sf::Packet packet;
            packet << Pkt::END_GAME;
            packet << tick << player.port;
            break;
        }
        default                     : {
            std::cout << "Unrecognized packet send in client section, please identify yourself " << packetTypeToSend << std::endl;
            break;
        }
    }
    packets.push_back(pkt); // Adds the packet to the array of packets.
    auto packetToSend = getLatestPacket();
    if (packetToSend.has_value()) {
        socket.send(packetToSend.value(), server, COMM_PORT_SERVER);
    }

    /*
        if (newRound) {
            sf::Packet packet;
            packet << Pkt::ACK << Pkt::ROUND_START;

            socket.send(packet, server, COMM_PORT_SERVER);

            newRound = false;
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
        */

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
            if (port == COMM_PORT_SERVER) {
                packet >> type;
                switch (type) {
                    case Pkt::SHUTDOWN      : {     // None                                                                         // indicate to the clients to shut down themselve
                        std::cout << "Client " << player.name << " received shutdown packet!" << std::endl;
                        loop = false;
                        break;
                    }
                    case Pkt::ACK           : {     // Pkt << tick                                                                  // make an acknoledge
                        packet >> typeAck;
                        switch(typeAck) {
                            case Pkt::NEW_PLAYER    : { // tick                                                                     // acknoledge the client that the server know he exist and have datas
                                if(created)break;
                                created = true;
                                // std::cout << "Player created ! "<< std::endl;
                                packetTypeToSend = Pkt::WAIT_START_R;
                                break;
                            }
                            default                 : {
                                std::cout << "Unrecognized acknoledge packet received in client section, please identify yourself " << typeAck << std::endl;
                                break;
                            }
                        }
                        break;
                    }
                    case Pkt::READY_R       : {
                        semaphore.acquire();

                        int tick;
                        int nbPlayers;

                        if (!(packet >> tick >> nbPlayers)) {
                            std::cout << "[CLIENT " << player.name << "] ERROR reading header" << std::endl;
                            semaphore.release();
                            break;
                        }

                        for (int i = 0; i < nbPlayers; i++) {
                            std::string n;
                            State s;
                            int r, g, b, a;
                            int wpn_id;

                            if (!(packet >> n >> r >> g >> b >> a >> wpn_id >> s)) {
                                std::cout << "[CLIENT " << player.name << "] ERROR reading player #" << i << std::endl;
                                break;
                            }

                            // PLAYER LOCAL
                            if (n == player.name) {
                                // bufferOnReceipt.addClient(player);
                                bufferOnReceipt.updateNextPlayerState(player, s);
                                if (!this->getCompensations()[Compensation::RECONCILIATION]) {
                                    this->player.position.setX(s.getPosition().getX());
                                    this->player.position.setY(s.getPosition().getY());
                                    this->player.radius = s.getRadius();
                                }
                                this->player.mode = s.getMode();
                                this->player.isAttacking = s.getAttack();
                                this->player.wpn.applyID(s.getWpn().getId());
                                this->player.point = s.getPoint();
                            }
                            // OPPONENT
                            else {
                                if (!opponents.contains(n)) {
                                    Player newOpponent;
                                    newOpponent.name = n;
                                    newOpponent.color = sf::Color(r, g, b, a);
                                    newOpponent.wpn.applyID(wpn_id);

                                    opponents[n] = newOpponent;

                                    bufferOnReceipt.addClient(opponents[n]);

                                    opponents[n].position.setX(s.getPosition().getX());
                                    opponents[n].position.setY(s.getPosition().getY());
                                    opponents[n].radius = s.getRadius();
                                    opponents[n].mode = s.getMode();
                                    opponents[n].isAttacking = s.getAttack();
                                    opponents[n].wpn.applyID(s.getWpn().getId());
                                    opponents[n].point = s.getPoint();
                                }

                                bufferOnReceipt.updateNextPlayerState(opponents[n], s);
                            }
                        }
                        bufferOnReceipt.push(tick);


                        // ===== ACK =====
                        packetTypeToSend = Pkt::ACK;
                        ackToSend = Pkt::READY_R;

                        semaphore.release();
                        break;
                    }
                    case Pkt::START_R       : {     // tick << amtPlayer                                                            // send the signal so start the fight to the players
                        std::cout << "Client " << getName() << " received ROUND_START" << std::endl;
                        semaphore.acquire();
                        packetTypeToSend = Pkt::INPUTS;
                        semaphore.release();
                        break;
                    }
                    case Pkt::GLOBAL        : {     // tick << amtPlayers << client.name << client.position << [...]                // send the position and the information about the players and it's opponent during the game
                        int nbPlayers;
                        int stateTick;
                        std::string name;
                        packet >> stateTick >> nbPlayers;

                        // std::cout<<player.name << " receive a global pket " <<std::endl;

                        while (nbPlayers > 0) {
                            State state;
                            packet >> name >> state;
                            std::cout<<"recieve packet to change " << name <<std::endl;
                            std::cout<< state.getPosition().getX() << " , " << state.getPosition().getY() << std::endl;

                            //TODO : make something to apply the state to yhe current players
                            // applyState(name, state);
                            nbPlayers--;
                        }

                        this->bufferOnReceipt.push(stateTick);
                        packet >> lastServerTick;

                        break;
                    }
                    case Pkt::DEATH         : {     // tick << killerName                                                           // send the signal to a specific player that the player is dead
                        // TODO
                        break;
                    }
                    case Pkt::END_R         : {     // tick                                                                         // send the signal that the round is finished
                        // TODO
                        break;
                    }
                    default                 : {
                        std::cout << "UNKNOWN PACKET RECEIVE CLIENT! Type: " << type << std::endl;
                        break;
                    }
                }

                // TODO : delete the old switch
                /*
                switch (type) {
                    case Pkt::ROUND_START: {
                        std::cout << "Client " << getName() << " received ROUND_START" << std::endl;
                        newRound = true;
                        // No break because round start has position afterwards (and therefor will execute Pkt::GLOBAL case)
                    }
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
                    case Pkt::POSITION: {
                        Position tempPos;
                        packet >> tempPos;
                        break;
                    }
                    case Pkt::SHUTDOWN: {
                        std::cout << "Client " << player.name << " received shutdown packet!" << std::endl;
                        loop = false;
                        break;
                    }

                    // TODO : [delete me while previous todo done] new packet

                    default:
                        std::cout << "UNKNOWN PACKET! Type: " << type << " for client " << player.name << std::endl;
                }
            */
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

            // TODO: Find a better fix. This just sets it back to old pos. without taking into account latest inputs.
            /* Should send a variable or something to tell that the position has been fixed, otherwise it gets fixed X times
             * Until the server sends back a new packet, causing those large "round-like" fixes.
            */
//            pos.setX(q.getX());
//            pos.setY(q.getY());
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
