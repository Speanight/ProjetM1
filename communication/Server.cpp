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

    player.weapons = {1, Weapons::SHIELD};

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
            if (port == COMM_PORT_SERVER) {
                packet >> type;

                switch (type) {
                    case Pkt::SHUTDOWN:
                        std::cout << "Received shutdown packet!" << std::endl;
                        loop = false;
                        break;

                    default:
                        std::cout << "UNKNOWN PACKET! Type: " << type << std::endl;
                        break;
                }
            }

            // Checks for all connected clients:
            for (auto &[name, player]: clients) {
                if (player.port == port) { // Check if ports corresponds (AKA the expected client)
                    packet >> type;

                    switch (type) {
                        case Pkt::ACK: {
                            int context;
                            packet >> context;

                            switch (context) {
                                case Pkt::ROUND_START:
                                    clients[name].status = Status::WAITING_FOR_ROUND_START;
                                    break;
                                case Pkt::ACK:
                                    clients[name].status = Status::READY_TO_START;
                                    break;
                                default:
                                    std::cout << "Received unknown ACK: " << context << " from client " << name << std::endl;
                            }
                            break;
                        }
                        case Pkt::INPUTS: {
                            int time;
                            packet >> time >> inputs;

                            // Get threads priority
                            semaphore.acquire();
                            addLine(
                                    name + " >>> Server [PING:" +
                                    std::to_string(clock.getElapsedTime().asMilliseconds() - time) + "ms] "
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
                            currentState = buffer.getCurrentState();
                            semaphore.acquire();
                            playerState = buffer.getLastState(player);
                            semaphore.release();
                            buffer.addInputsToLastState(player, clock.getElapsedTime().asMilliseconds(), inputs);

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


                                    if (inputs.getAttack()) {
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

                                    if (interaction) {
                                        State s = State(clock.getElapsedTime().asMilliseconds(), opponentPos, inputs,
                                                        currentState[n].getRadius(), currentState[n].getAttack(),
                                                        currentState[n].getWpn().getId(), currentState[n].getPoint());
                                        buffer.updateNextPlayerState(p, s);
                                    }
                                }

                                semaphore.acquire();
                                State s = State(time, position, inputs, radius, attack, wpn_id, playerState.getPoint());
                                player.position = position;
                                player.radius = radius;
                                player.isAttacking = attack;
                                player.wpn = Weapon(wpn_id);
                                player.point = playerState.getPoint();
                                buffer.updateNextPlayerState(player, s);
                                semaphore.release();
                            }
                            break;
                        }
                        default: {
                            std::cout << " UNKNOWN CLIENT PACKET! Type: " << type << " from client " << name << std::endl;
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
    unsigned int timeRoundStart = 0;
    unsigned short playersReady = 0;

    while (loop) {
        std::unordered_map<std::string, State> currentState = buffer.getCurrentState();

        for (auto & [name, player] : clients) {
            packet.clear();
            if (playersReady == clients.size() or timeRoundStart != 0) {
                clients[name].status = Status::DONE;
                timeRoundStart = buffer.getCurrentTick();
                playersReady = 0;
            }
            switch (player.status) {
                // If player is still waiting for round start packet:
                case Status::WAITING_FOR_ROUND_START: {
                    packet << Pkt::ROUND_START << int(buffer.getCurrentTick()) << int(clients.size());

                    int playerNb = clients.size();
                    int time = clock.getElapsedTime().asMilliseconds();

                    for (auto & [name, player] : clients) {
                        Position pos;
                        pos.setX((playerNb * Const::MAP_SIZE_X / (clients.size())) - (Const::MAP_SIZE_X / clients.size()) / 2);
                        pos.setY(Const::MAP_SIZE_Y / 2);
                        Input inputs(buffer.getLastState(player).getInputs().end()->second.getId(), 0, 0, 0.f, false, false);
                        State s = State(time, pos, inputs, std::numbers::pi/2, true);
                        buffer.updateNextPlayerState(player, s);

                        playerNb--;
                    }
                    buffer.push(clock.getElapsedTime().asMilliseconds());
                    currentState = buffer.getCurrentState(); // Refresh current state.
                    break;
                }

                // If player is ready to start (and waiting for other player):
                case Status::READY_TO_START:
                    playersReady++;
                    packet << Pkt::ACK << Pkt::ACK << int(buffer.getCurrentTick()) << int(clients.size());
                    break;

                // Everyone is ready to start, therefor normal behavior must happen:
                case Status::DONE: {
                    packet << Pkt::GLOBAL << int(buffer.getCurrentTick()) << int(currentState.size());

                    for (auto & [n, state] : currentState) {
                        packet << n << state;
                    }
                }
            }

            semaphore.acquire();
            packet << clock.getElapsedTime().asMilliseconds(); // Sync clocks
            socket.send(packet, sender.value(), player.port);

            addLine(
                "Server >>> " + name
                + " position: (" + std::to_string(player.position.getX())
                + ", " + std::to_string(player.position.getY())
                + ") ; radius: " + std::to_string(player.radius)
                + " Weapon: " + std::to_string(player.wpn.getId())
                + " attack: " + std::to_string(player.isAttacking)
                , sf::Color::White);
            semaphore.release();
        }

        semaphore.acquire();
        addToGraph(clock.getElapsedTime().asMilliseconds(), "Server", "clients");
        buffer.push(clock.getElapsedTime().asMilliseconds());
        semaphore.release();

        // POST MAJ
        for (auto & [name, player] : clients) {
            State last = buffer.getLastState(player);
            if(last.getAttack()) {          // setting the attack save into false one so we don't keep the attack signal
                last.setAttack(false);
                buffer.updateNextPlayerState(player, last);
            }
        }
        sf::sleep(tickrate);
    }

    receiveThread.join();
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

    if (socket.send(packet, sender.value(), COMM_PORT_SERVER) == sf::Socket::Status::Done) {
        // Sends DC packet to all clients
        for (auto & [name, player] : clients) {
            socket.send(packet, sender.value(), player.port);
        }
        return Err::ERR_NONE;
    }

    return Err::ERR_SERVER_SHUTDOWN;
}
