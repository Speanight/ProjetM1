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

Server::Server(const sf::Clock clock, int maxPlayers) : semaphore(1) {
    this->clock = clock;
    this->maxPlayers = maxPlayers;
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
    int typeAck;
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
            // Checks for all connected clients:
            // std::cout<<"SERVER RECEIVE A PACKET"<<std::endl;
            packet >> type;
            if (type == Pkt::NEW_PLAYER) {
                // tick << name << r g b a << wpn << port
                int tick;
                std::string pname;
                short r, g, b, a;
                int wpn_id;
                short clientPort;

                packet >> tick >> pname >> r >> g >> b >> a >> wpn_id >> clientPort;

                sf::Color color(r, g, b, a);

                semaphore.acquire();
                if (!clients.contains(pname)) {
                    std::cout << "NEW PLAYER: " << pname << std::endl;

                    // ===== PREPARE DATA FOR addClient =====
                    std::unordered_map<std::string, std::any> infos;
                    infos["error"] = false;
                    infos["name"]  = pname;
                    infos["port"] = static_cast<unsigned short>(clientPort);
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
                    buffer.push(time);

                }

                // ===== ACK =====
                packetTypeToSend = Pkt::ACK;
                ackToSend = Pkt::NEW_PLAYER;

                semaphore.release();
            }
            else {
                for (auto &[name, player]: clients) {
                    if (player.port == port) { // Check if ports corresponds (AKA the expected client)
                        switch (type) {
                            case Pkt::SHUTDOWN      : {
                                std::cout << "Received shutdown packet!" << std::endl;
                                loop = false;
                                break;
                            }
                            case Pkt::ACK           : {     // Pkt << typeAck << tick
                                int tick;
                                packet >> typeAck >> tick;

                                switch(typeAck) {
                                    case Pkt::READY_R   : {
                                        // std::cout << "THE WAR MAY BEGIN ..." << std::endl;
                                        semaphore.acquire();
                                        packetTypeToSend = Pkt::START_R;
                                        semaphore.release();
                                        break;
                                    }
                                    case Pkt::DEATH     : {
                                        // TODO
                                        break;
                                    }
                                    case Pkt::END_R     : {
                                        // TODO
                                        break;
                                    }
                                    default             : {
                                        std::cout << "Unrecognized ack packet: " << typeAck << std::endl;
                                        break;
                                    }
                                }
                                break;
                            }
                            case Pkt::WAIT_START_R  : {
                                // tick << port
                                int tick;
                                short clientPort;

                                packet >> tick >> clientPort;

                                // std::cout << clientPort << " said he was ready to start" << std::endl;
                                if (clients.size() < maxPlayers) {
                                    std::cout << "Not enough players yet" << std::endl;
                                    semaphore.acquire();
                                    packetTypeToSend = Pkt::NONE;
                                    semaphore.release();
                                    break;
                                }
                                // std::cout<<"WE CAN START ! "<<std::endl;
                                semaphore.acquire();
                                packetTypeToSend = Pkt::READY_R;
                                semaphore.release();
                                break;
                            }
                              case Pkt::INPUTS        : {
                                  // tick << inputs << port
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
                                State s = State(
                                    time,
                                    position,
                                    inputs,
                                    radius,
                                    attack,
                                    wpn_id,
                                    playerState.getPoint()
                                );
                                buffer.updateNextPlayerState(player, s);
                                semaphore.release();
                            }
                                  semaphore.acquire();
                                  packetTypeToSend = Pkt::GLOBAL;
                                  semaphore.release();
                                  break;
                              }
                            case Pkt::END_GAME      : {
                                int tick;
                                short clientPort;

                                packet >> tick >> clientPort;

                                std::cout << "END GAME received" << std::endl;
                                loop = false;
                                break;
                            }
                            default                 : {
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
    unsigned int timeRoundStart = 0;
    unsigned short playersReady = 0;

    while (loop) {
        sf::sleep(tickrate);

        std::unordered_map<std::string, State> currentState = buffer.getCurrentState();

        int tick = clock.getElapsedTime().asMilliseconds();

        for (auto & [name, player] : clients) {
            sf::Packet packet;
            switch (packetTypeToSend) {
                case Pkt::NONE          : {
                    break;
                }
                case Pkt::SHUTDOWN      : {     // None
                    packet << Pkt::SHUTDOWN;
                    break;
                }
                case Pkt::ACK           : {     // Pkt << tick
                    packet << Pkt::ACK;
                    switch(ackToSend) {
                        case Pkt::NEW_PLAYER    : {
                            packet << Pkt::NEW_PLAYER << tick;
                            break;
                        }
                        default                 : {
                            std::cout << "Unrecognized acknowledge packet sent in server section, please identify yourself " << ackToSend << std::endl;
                            break;
                        }
                    }
                    break;
                }
                case Pkt::READY_R       : {
                    packet << Pkt::READY_R;

                    int nbPlayers = static_cast<int>(clients.size());
                    packet << tick << nbPlayers;

                    semaphore.acquire();
                    int playerNb = maxPlayers;

                    for (auto & [n, p] : clients) {
                        Position pos;
                        pos.setX((playerNb * Const::MAP_SIZE_X / maxPlayers)- (Const::MAP_SIZE_X / maxPlayers) / 2);
                        pos.setY(Const::MAP_SIZE_Y / 2);

                        auto inputsMap = buffer.getLastState(p).getInputs();
                        int lastInputId = 0;

                        if (!inputsMap.empty()) {
                            lastInputId = inputsMap.rbegin()->second.getId();
                        }

                        Input inputs(lastInputId, 0, 0, 0.f, true, false, 0);

                        State s(tick, pos, inputs, std::numbers::pi/2, true, 0, 100);
                        buffer.updateNextPlayerState(p, s);

                        packet
                            << n
                            << int(p.color.r)
                            << int(p.color.g)
                            << int(p.color.b)
                            << int(p.color.a)
                            << p.wpn.getId()
                            << s
                        ;

                        playerNb--;
                    }

                    buffer.push(tick);
                    semaphore.release();

                    break;
                }
                case Pkt::START_R       : {     // tick << amtPlayer
                    packet << Pkt::START_R;
                    packet << tick;
                    break;
                }
                case Pkt::GLOBAL        : {     // tick << amtPlayers << states
                    packet << Pkt::GLOBAL;
                    packet << tick;
                    int nbClients;
                    std::unordered_map<std::string, State> safeState;

                    semaphore.acquire();
                    nbClients = clients.size();
                    safeState = currentState;
                    semaphore.release();

                    packet << nbClients;
                    for (auto & [n, state] : safeState) {
                        packet << n << state;
                    }
                    break;
                }
                case Pkt::DEATH         : {     // tick << killerName
                    packet << Pkt::DEATH;
                    packet << tick;
                    // packet << lastKillerName;
                    break;
                }
                case Pkt::END_R         : {     // tick
                    packet << Pkt::END_R;
                    packet << tick;
                    break;
                }
                default                 : {
                    std::cout << "UNKNOWN PACKET! Type: " << packetTypeToSend << std::endl;
                    continue;
                }
            }

            if(packetTypeToSend != Pkt::NONE) {
                // PRINT MESSAGE IN SERVER CONSOLE
                addLine(
                    "Server >>> " + name
                    + " position: (" + std::to_string(player.position.getX())
                    + ", " + std::to_string(player.position.getY())
                    + ") ; radius : " + std::to_string(player.radius)
                    + " attack : " + std::to_string(player.isAttacking),
                    sf::Color::White
                );

                // BUFFER UPDATE
                semaphore.acquire();
                addToGraph(clock.getElapsedTime().asMilliseconds(), "Server", "clients");
                buffer.push(clock.getElapsedTime().asMilliseconds());
                semaphore.release();

                // POST MAJ
                for (auto & [name, player] : clients) {
                    State last = buffer.getLastState(player);
                    if (last.getAttack()) {
                        last.setAttack(false);
                        buffer.updateNextPlayerState(player, last);
                    }
                }

                // SENDING TO CLIENT
                // std::cout << "sending to clients " << packetTypeToSend <<std::endl;
                semaphore.acquire();
                socket.send(packet, sender.value(), player.port);
                semaphore.release();
            }
            else {
                std::cout << "waiting for players" << std::endl;
            }
        }

    }

    receiveThread.join();
    return Err::ERR_NONE;
}

/*
                switch (partyState) {
                    case PartyState::GAME_START : {
                        printf("starting the game");
                    }
                    case PartyState::GAME_STOP : {
                        printf("ending the game");
                    }
                    case PartyState::RUN_START : {
                        printf("starting the run");
                        player.status = Status::WAITING_FOR_ROUND_START;
                        packet << Pkt::ROUND_START << int(buffer.getCurrentTick()) << int(clients.size());

                int playerNb = clients.size();
                int time = clock.getElapsedTime().asMilliseconds();

                for (auto & [name, player] : clients) {
                    Position pos;
                    pos.setX((playerNb * Const::MAP_SIZE_X / (clients.size())) - (Const::MAP_SIZE_X / clients.size()) / 2);
                    pos.setY(Const::MAP_SIZE_Y / 2);
                    Input inputs(buffer.getLastState(player).getInputs().end()->second.getId(), 0, 0, 0.f, false, false, 0);
                    State s = State(time, pos, std::numbers::pi/2, true, false, 0, 0, inputs);
                    buffer.updateNextPlayerState(player, s);

                    playerNb--;
                }
                buffer.push(clock.getElapsedTime().asMilliseconds());
                currentState = buffer.getCurrentState(); // Refresh current state.
            }
            else {
                packet << Pkt::GLOBAL << int(buffer.getCurrentTick()) << int(currentState.size());
            }

            for (auto & [n, state] : currentState) {
                packet << n << state;
            }

            semaphore.acquire();
            packet << clock.getElapsedTime().asMilliseconds(); // Sync clocks
            socket.send(packet, sender.value(), player.port);

            addLine(
                "Server >>> " + name
                + " position: (" + std::to_string(player.position.getX())
                + ", " + std::to_string(player.position.getY())
                + ") ; radius : " + std::to_string(player.radius)
                + " Weapon: " + std::to_string(player.wpn.getId())
                + " attack : " + std::to_string(player.isAttacking)
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

    loop = false;

    return Err::ERR_SERVER_SHUTDOWN;
}

int Server::getMaxPlayers() {
    return maxPlayers;
}

