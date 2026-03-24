#include "Server.hpp"

/**
 * A server is being initialized with MainWindow. It's a needed component to ensure communication between the different
 * clients. It receives the queuedPackets, and send queuedPackets back to each client. For that, they need to be added manually
 * after creation of the server.
 *
 * @param clock Clock, needed to synchronise clients and server together for packet transmission.
 */
Server::Server(const sf::Clock clock) : semaphore(1) {
    this->clock = clock;
    if (socket.bind(COMM_PORT_SERVER) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available?" << std::endl;
    }
    else {
        socket.setBlocking(true);
        sendThread = std::thread(&Server::sendLoop, this);
        receiveThread = std::thread(&Server::receiveLoop, this);
    }
}

Server::~Server() {
    socket.unbind();
    if (sendThread.joinable()) {
        sendThread.join();
    }
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
}

/**
 * Function that allows to get info of clients paired with the server.
 *
 * @return map of clients, keys being their name and values being their port.
 */
std::unordered_map<std::string, Player> Server::getClients() {
    return clients;
}

/**
 * Allows to add clients to the server's tracked routes. This means the server will send and receive queuedPackets from the
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
    Player player;
    player.port = std::any_cast<unsigned short>(infos["port"]);
    player.name = std::any_cast<std::string>(infos["name"]);
    player.color = std::any_cast<sf::Color>(infos["color"]);
    Weapon wpn(std::any_cast<int>(infos["wpn_id"]));
    player.wpn = wpn;

    player.weapons = {1, Weapons::SHIELD};

    player.status = Status::WAITING_FOR_INIT;

    clients[std::any_cast<std::string>(infos["name"])] = player;
    pings[std::any_cast<std::string>(infos["name"])] = 0;
    buffer.addClient(player);
    std::cout << "Added client " << std::any_cast<std::string>(infos["name"]) << " on port " << clients[std::any_cast<std::string>(infos["name"])].port << std::endl;
    addToData(std::any_cast<std::string>(infos["name"]));
    return Err::ERR_NONE;
}

/**
 * Loop that executes every tick rate: Server will calculate position of client if incorrect/impossible. Recovers
 * positions of clients. This function shouldn't return, except if the server stops.
 *
 *  @return int code for info as to how the server ended (type Err::)
 */
int Server::receiveLoop() {
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve("127.0.0.1");
    sf::Packet packet;
    short unsigned int port;
    int type;
    Position position;
    Input inputs;
    const sf::Time tickrate = std::chrono::milliseconds(TICKRATE);
    int dt;
    std::unordered_map<std::string, State> currentState;
    State playerState;
    float radius;

    while (loop) {
        sf::sleep(sf::Time());
        packet.clear();

        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {
            packet >> type;
            if (type == Pkt::NEW_PLAYER) {
                // tick << name << r g b a << wpn
                int tick;
                std::string pname;
                short r, g, b, a;
                int wpn_id;

                packet >> pname >> r >> g >> b >> a >> wpn_id;

                a = 255;

                sf::Color color(r, g, b, a);

                if (!clients.contains(pname)) {
                    // ===== PREPARE DATA FOR addClient =====
                    std::unordered_map<std::string, std::any> infos;
                    infos["error"] = false;
                    infos["name"]  = pname;
                    infos["port"] = port;
                    infos["color"] = color;
                    infos["wpn_id"] = wpn_id;

                    addClient(infos);

                    // ===== INIT STATE =====
                    int time = clock.getElapsedTime().asMilliseconds();

                    Input defaultInput(0, 0.f, 0.f, 0.f, false, false, wpn_id);

                    State initialState(
                        time,
                        Position(0, 0),
                        defaultInput,
                        0.f,
                        true,
                        wpn_id,
                        100
                    );
                }

                clients[pname].status = Status::WAITING_FOR_INIT;

                // ===== ACK =====
                packetTypeToSend = Pkt::ACK;
                ackToSend = Pkt::NEW_PLAYER;
            }
            else if (port == COMM_PORT_SERVER) {
                switch (type) {
                    case Pkt::SHUTDOWN: {
                        std::cout << "Received shutdown packet!" << std::endl;
                        loop = false;
                        break;
                    }
                }
            }
            else {
                for (auto &[name, player]: clients) {
                    if (player.port == port) { // Check if ports corresponds (AKA the expected client)
                        switch (type) {
                            case Pkt::ACK: {
                                int typeAck;
                                packet >> typeAck;

                                switch (typeAck) {
                                    // User knows it has been ACKd, and is waiting to know their opponents.
                                    case Pkt::NEW_PLAYER: {
                                        clients[name].status = Status::WAITING_FOR_OPPONENTS;
                                        break;
                                    }

                                    // User received opponents info and is waiting for the start signal:
                                    case Pkt::READY_R: {
                                        clients[name].status = Status::READY_TO_START;
                                        break;
                                    }

                                    default: {
                                        std::cout << "Server received unknown ACK from client " << name << ": ack #" << typeAck << std::endl;
                                    }
                                }
                                break;
                            }

                            case Pkt::INPUTS: {
                                clients[name].status = Status::DONE;
                                int time;
                                packet >> time >> inputs;

                                // Get threads priority
                                semaphore.acquire();
                                addLine(
                                    name + " >>> Server [PING:" + std::to_string(clock.getElapsedTime().asMilliseconds() - time) +"ms] "
                                    + " | inputs: x=" + std::to_string(inputs.getMovementX()) +
                                    "; y=" + std::to_string(inputs.getMovementY()) +
                                    "; rotate = " + std::to_string(inputs.getRotate()) +
                                    "; changeWpn = " + std::to_string(inputs.getChangeWpn()) +
                                    "; attack = " + std::to_string(inputs.getAttack()) +
                                    "; inputs #" + std::to_string(inputs.getId()),
                                    player.color
                                );
                                addToGraph(clock.getElapsedTime().asMilliseconds(), name, "Server");
                                semaphore.release();

                                // Updates ping of corresponding client:
                                pings[player.name] = clock.getElapsedTime().asMilliseconds() - time;

                                // Get the current server state AND last player state (which might be the next server state!)
                                semaphore.acquire();
                                currentState = buffer.getCurrentState();
                                playerState = buffer.getLastState(player);
                                buffer.addInputsToLastState(player, clock.getElapsedTime().asMilliseconds(), inputs);
                                semaphore.release();

                                // ====== POSITION ======
                                position = playerState.getPosition();
                                radius = playerState.getRadius();

                                // Get time elapsed since last packet from client. Used for consistency in speed and such.
                                dt = std::min(time - playerState.getTimestamp(), static_cast<int>(Const::TICKRATE.count()));

                                // Adjust client values according to last state and new inputs values.
                                position.move(inputs.getMovementX(), inputs.getMovementY(), dt);
                                playerState.setPosition(position);

                                if (inputs.getOnController()) { // If inputs are made through R-stick of controller:
                                    radius = inputs.getRotate(); // Get raw inputs
                                }
                                else { // Otherwise calculate with rotate speed:
                                    radius += std::fmod(inputs.getRotate() * Const::PLAYER_RADIUS_SPEED * dt, 2.f * std::numbers::pi);
                                }

                                // ====== ATTACK ======
                                bool attack = inputs.getAttack() or playerState.getAttack();

                                // ====== WEAPON DATAS CHANGE ======
                                if (inputs.getChangeWpn()) {
                                    player.weapon = (player.weapon + 1) % player.weapons.size();
                                    player.wpn = Weapon(player.weapons[player.weapon]);
                                }

                                int wpn_id = player.weapons[player.weapon];
                                currentState[name].setWpn(wpn_id);

                                // loops of all interaction between players
                                for (auto &[n, p]: clients) {
                                    if (name != n) {
                                        bool interaction = false;
                                        // Check if there is a collision between players (and therefor if it should be resolved)
                                        Position opponentPos = currentState[n].getPosition();
                                        opponentPos = resolveCollision(position, opponentPos);

                                        // If yes, we re-adjust the new position of said opponent:
                                        if (opponentPos != currentState[n].getPosition()) {
                                            interaction = true;
                                        }


                                        if(inputs.getAttack()) {
                                            interaction = true;
                                            semaphore.acquire();
                                            State stO = buffer.getStateAtTimestamp(p, clock.getElapsedTime().asMilliseconds() - pings[player.name] - pings[p.name] - Const::TICKRATE.count());
                                            semaphore.release();

                                            // Process the attack only if value in buffer (aka. max amt. of lag taken into consideration)
                                            if (stO.getTimestamp() != 0) {
                                                short attackResult = resolveAttacks(playerState, stO);

                                                // If attack has been blocked:
                                                if (attackResult == 0) {
                                                    // HIT SECTION
                                                    int pts = currentState[name].getPoint() + 1;
                                                    currentState[name].setPoint(pts);
                                                    playerState.setPoint(pts);
                                                }
                                                // If attack has not been blocked and is hitting:
                                                else if (attackResult == 1) {
                                                    // BLOCKED SECTION
                                                    float dirx = std::cos(radius);
                                                    float diry = std::sin(radius);

                                                    // distance de knockback = 2x la range de l'arme
                                                    float knockbackDist = currentState[name].getWpn().getRange() * 2.f;

                                                    // nouvelle position du joueur attaquant
                                                    position.setX(position.getX() - dirx * knockbackDist);
                                                    position.setY(position.getY() - diry * knockbackDist);
                                                }
                                            }
                                        }

                                        if(interaction) {
                                            State s = State(clock.getElapsedTime().asMilliseconds(),
                                                opponentPos, inputs,
                                                currentState[n].getRadius(),
                                                currentState[n].getAttack(),
                                                currentState[n].getWpn().getId(),
                                                currentState[n].getPoint());
                                            buffer.updateNextPlayerState(p, s);
                                        }
                                    }

                                    semaphore.acquire();
                                    State s = State(time, position, inputs, radius,
                                        attack, wpn_id, playerState.getPoint());
                                    buffer.updateNextPlayerState(player, s);
                                    semaphore.release();
                                }
                                semaphore.acquire();
                                packetTypeToSend = Pkt::GLOBAL;
                                semaphore.release();
                                break;
                            }
                            case Pkt::END_GAME: {
                                int tick;
                                short clientPort;

                                packet >> tick >> clientPort;

                                std::cout << "END GAME received" << std::endl;
                                loop = false;
                                break;
                            }
                            default: {
                                std::cout << "UNKNOWN PACKET TO SEND! Type: " << type << std::endl;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    sendThread.join();
    return Err::ERR_NONE; // Exited without any issue.
}

/**
 * Loop that executes every tick rate: Needed info will be sent to the clients whenever needed. This is in a while loop.
 *
 *  @return int code for info as to how the server ended (type Err::)
 */
int Server::sendLoop() {
    const sf::Time tickrate = std::chrono::milliseconds(TICKRATE);
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve("127.0.0.1");
    sf::Packet packet;

    while (loop) {
        sf::sleep(tickrate);

        std::unordered_map<std::string, State> currentState = buffer.getCurrentState();

        int tick = clock.getElapsedTime().asMilliseconds();

        for (auto & [name, player] : clients) {
            sf::Packet packet;
            switch (player.status) {
                // If user has been created but didn't receive confirmation yet:
                case Status::WAITING_FOR_INIT: {
                    packet << Pkt::ACK << Pkt::NEW_PLAYER;
                    break;
                }

                // If user has been ACKd, and is waiting to know their opponents:
                case Status::WAITING_FOR_OPPONENTS: {
                    bool ready = false;
                    // Check if everyone is ready first:
                    if (clients.size() == amtPlayers) {
                        ready = true;
                        packet << Pkt::READY_R;
                        int nb = 1;
                        for (auto &[n, p] : clients) {
                            // Check that everyone know they've been ACKd by server:
                            ready = ready and p.status == Status::WAITING_FOR_OPPONENTS;

                            // Set position of clients:
                            Position pos;
                            pos.setX(Const::MAP_SIZE_X / 2 + (Const::MAP_SIZE_X / 4) * std::cos(nb*(2*M_PI / clients.size())));
                            pos.setY(Const::MAP_SIZE_Y / 2 + (Const::MAP_SIZE_Y / 4) * std::sin(nb*(2*M_PI / clients.size())));

                            Input inputs;

                            State s(tick, pos, inputs, std::numbers::pi/2, true, 0, 100);
                            buffer.updateNextPlayerState(p, s);

                            // Add everything in packet:
                            packet << p.name << p.color.r << p.color.g << p.color.b << p.color.a << s;
                            nb++;
                        }
                    }
                    else {
                        std::cout << "Received " << clients.size() << "/" << amtPlayers << " players." << std::endl;
                    }

                    // If not ready, send an ACK that client is waiting:
                    if (!ready) {
                        packet.clear();
                        packet << Pkt::ACK << Pkt::WAIT_OPPONENTS;
                    }
                    break;
                }

                // User has been ACKd by server, and user also has opponents added:
                case Status::READY_TO_START: {
                    // Check if everyone is ready to start:
                    bool ready = true;
                    for (auto &[n, p] : clients) {
                        ready = ready and p.status == Status::READY_TO_START;
                    }

                    if (ready) {
                        packet << Pkt::GLOBAL << int(buffer.getCurrentTick());

                        for (auto & [n, state] : currentState) {
                            packet << n << state;
                        }
                    }
                    else {
                        packet << Pkt::ACK << Pkt::WAIT_START_R;
                    }
                    break;
                }

                case Status::DONE: {
                    packet << Pkt::GLOBAL << clock.getElapsedTime().asMilliseconds();
                    for (auto & [n, state] : currentState) {
                        packet << n << state;
                    }
                    buffer.push(clock.getElapsedTime().asMilliseconds());
                }
            }

            socket.send(packet, sender.value(), player.port);
        }
    }
    receiveThread.join();
    return Err::ERR_NONE;
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

    if (socket.send(packet, sender.value(), COMM_PORT_SERVER) == sf::Socket::Status::Done) {
        // Sends DC packet to all clients
        for (auto & [name, player] : clients) {
            socket.send(packet, sender.value(), player.port);
        }
        return Err::ERR_NONE;
    }

    loop = false;

    return Err::ERR_SERVER_SHUTDOWN;
}