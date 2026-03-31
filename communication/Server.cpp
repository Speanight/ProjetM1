#include "Server.hpp"

/**
 * A server is being initialized with MainWindow. It's a needed component to ensure communication between the different
 * clients. It receives the queuedPackets, and send queuedPackets back to each client. For that, they need to be added manually
 * after creation of the server.
 *
 * @param clock Clock, needed to synchronise clients and server together for packet transmission.
 */
Server::Server(Console &console, sf::Clock& clock) : ServerUI(console), semaphore(1), clock(clock) {
    if (socket.bind(COMM_PORT_SERVER) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available?" << std::endl;
    }
    else {
        console.addClient(COMM_PORT_SERVER);
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


int Server::getMapID() {
    return mapID;
}


/**
 * Allows to add clients to the server's tracked routes. This means the server will send and receive queuedPackets from the
 * port of the clients added.
 *
 * @param infos map of infos, usually returned by Client::init().
 * @return Error code
 */
int Server::addClient(const std::string& name, unsigned short port, sf::Color color, short weapon) {
    Player player;
    player.setPort(port);
    player.setName(name);
    player.setColor(color);
    player.setWeapons({Weapons::SHIELD, weapon});

    clients[port] = player;
    pings[name] = 0;
    buffer.addClient(player);
    console.addClient(port);
    std::cout << "Added client " << name << " on port " << port << std::endl;
    addToData(name);
    return Err::ERR_NONE;
}

/**
 * Loop that executes every tick rate: Server will calculate position of client if incorrect/impossible. Recovers
 * positions of clients. This function shouldn't return, except if the server stops.
 *
 *  @return int code for info as to how the server ended (type Err::)
 */
[[noreturn]] void Server::receiveLoop() {
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve("127.0.0.1");
    sf::Packet packet;
    short unsigned int port;
    short type;
    Position position;
    Input inputs;
    int dt;
    std::unordered_map<std::string, State> currentState;
    State playerState;
    float radius;
    uint32_t id;

    while (true) {
        while (!loop) {sf::sleep(sf::Time());} // Pause if needed
        sf::sleep(sf::Time());
        packet.clear();

        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {
            packet >> id >> type;
            if (type == Pkt::NEW_PLAYER) {
                // name << r g b a << wpn
                std::string pname;
                std::uint8_t r, g, b, a;
                short wpn_id;

                packet >> pname >> r >> g >> b >> a >> wpn_id;

                sf::Color color(r, g, b, a);

                // Check if player exists:
                semaphore.acquire();
                if (!clients.empty()) {
                    for (auto & [playerPort, player] : clients) {
                        // If port corresponds (aka same client:)
                        if (playerPort == port and player.getStatus() != Status::WAITING_FOR_INIT) {
                            buffer.removeFromPlayerList(player);
                            removeToData(player.getName());
                            clients.erase(playerPort); // Delete it.
                            break;
                        }
                    }
                }

                addClient(pname, port, color, wpn_id);

                clients[port].setStatus(Status::WAITING_FOR_INIT);
                semaphore.release();
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
            else if (clients.contains(port)) {
                Player player = clients[port];
                switch (type) {
                    case Pkt::ACK: {
                        short typeAck;
                        packet >> typeAck;

                        switch (typeAck) {
                            // User knows it has been ACKd, and is waiting to know their opponents.
                            case Pkt::NEW_PLAYER: {
                                clients[port].setStatus(Status::WAITING_FOR_OPPONENTS);
                                break;
                            }

                            // User received opponents info and is waiting for the start signal:
                            case Pkt::READY_R: {
                                int nbOpp;
                                packet >> nbOpp;

                                if (nbOpp == clients.size() - 1) {
                                    clients[port].setStatus(Status::READY_TO_START);
                                }
                                else {
                                    std::cout << port << " is missing one (or more) opponents!" << std::endl;
                                    clients[port].setStatus(Status::WAITING_FOR_OPPONENTS);
                                }
                                break;
                            }
                            case Pkt::DEATH: {
                                // Counting for the remaining survivors
                                int nbPlayers = maxPlayers;
                                for (auto &[n, p]: clients) {
                                    if(p.getStatus() == Status::DEAD) {
                                        nbPlayers --;
                                    }
                                }
                                if(nbPlayers == 1) {
                                    clients[port].setStatus(Status::DEAD);
                                }
                                break;
                            }
                            case Pkt::END_R: {
                                std::cout<<"receive end_r ack" << std::endl;
                                // TODO : delete actual players in the buffer and put itself in "waiting for players mode" / putting packet typeToSend to "None"
                                break;
                            }
                            default: {
                                std::cout << "Server received unknown ACK from client " << port << ": ack #" << typeAck << std::endl;
                            }
                        }
                        break;
                    }

                    case Pkt::INPUTS: {
                        if (clients[port].getStatus() != Status::DEAD) {
                            clients[port].setStatus(Status::DONE);
                        }
                        int time;
                        packet >> time >> inputs;

                        // Get threads priority
                        semaphore.acquire();
                        addLine(
                            clients[port].getName() + " >>> Server [PING:" + std::to_string(clock.getElapsedTime().asMilliseconds() - time) +"ms] "
                            + " | inputs: x=" + std::to_string(inputs.getMovementX()) +
                            "; y=" + std::to_string(inputs.getMovementY()) +
                            "; rotate = " + std::to_string(inputs.getRotate()) +
                            "; changeWpn = " + std::to_string(inputs.getChangeWpn()) +
                            "; attack = " + std::to_string(inputs.getAttack()) +
                            "; inputs #" + std::to_string(inputs.getId()),
                            player.getColor()
                        );
                        addToGraph(clock.getElapsedTime().asMilliseconds(), player.getName(), "Server");
                        semaphore.release();

                        // Updates ping of corresponding client:
                        pings[player.getName()] = clock.getElapsedTime().asMilliseconds() - time;

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
                        dt = std::min(time - playerState.getTimestamp(), 1000/static_cast<int>(tickrate));

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
                            clients[port].switchWeapon();
                        }
                        currentState[player.getName()].setWpn(player.getWpn().getId());

                        // loops of all interaction between players
                        for (auto &[plPort, p]: clients) {
                            if (port != plPort) {
                                bool interaction = false;
                                // Check if there is a collision between players (and therefor if it should be resolved)
                                Position opponentPos = currentState[p.getName()].getPosition();
                                opponentPos = resolveCollision(position, opponentPos);

                                // If yes, we re-adjust the new position of said opponent:
                                if (opponentPos != currentState[p.getName()].getPosition()) {
                                    interaction = true;
                                }


                                // If user clicks attack button AND not in attack animation still:
                                if(inputs.getAttack() and clock.getElapsedTime().asMilliseconds() - playerState.getAttackTimestamp() >= (player.getWpn().getAttackSpeed() + player.getWpn().getReload())) {
                                    interaction = true;
                                    semaphore.acquire();
                                    State stO;
                                    if (rewind) {
                                        stO = buffer.getStateAtTimestamp(p, clock.getElapsedTime().asMilliseconds() - pings[player.getName()] - pings[p.getName()] - tickrate);
                                    }
                                    else {
                                        stO = buffer.getLastState(p);
                                    }
                                    semaphore.release();

                                    // Process the attack only if value in buffer (aka. max amt. of lag taken into consideration)
                                    if (stO.getTimestamp() != 0) {
                                        short attackResult = resolveAttacks(playerState, stO);

                                        // If attack has not been blocked and is hitting:
                                        if (attackResult == 0) {
                                            // HIT SECTION
                                            // TODO : section that choose how the point react depending on the game type we are
                                            semaphore.acquire();
                                            auto opponent = buffer.getLastState(p.getName());
                                            semaphore.release();

                                            if(demo_mode) {
                                                int pts = currentState[player.getName()].getPoint() + 1;
                                                currentState[player.getName()].setPoint(pts);
                                                playerState.setPoint(pts);
                                            }
                                            else {
                                                int pts = opponent.getPoint()-currentState[player.getName()].getWpn().getDamage();
                                                if(pts <= 0) {
                                                    pts = 0;
                                                    clients[plPort].setStatus(Status::DEAD);
                                                }
                                                currentState[p.getName()].setPoint(pts);
                                                opponent.setPoint(pts);
                                            }
                                        }
                                        // If attack has been blocked:
                                        else if (attackResult == 1) {
                                            // BLOCKED SECTION
                                            float dirx = std::cos(radius);
                                            float diry = std::sin(radius);

                                            // distance de knockback = 2x la range de l'arme
                                            float knockbackDist = currentState[p.getName()].getWpn().getRange() * 2.f;

                                            // nouvelle position du joueur attaquant
                                            position.setX(position.getX() - dirx * knockbackDist);
                                            position.setY(position.getY() - diry * knockbackDist);
                                        }
                                    }
                                }

                                if(interaction) {
                                    // TODO: Fix attack that "rewinds" player back few pos. Perhaps "display it" if ticked server-wise?
                                    State s = State(currentState[p.getName()].getTimestamp(),
                                        opponentPos, inputs,
                                        currentState[p.getName()].getRadius(),
                                        currentState[p.getName()].getAttack(),
                                        currentState[p.getName()].getWpn().getId(),
                                        currentState[p.getName()].getPoint());

                                    // TODO: Find better way to do it:
                                    if (opponentPos != currentState[p.getName()].getPosition()) {
                                        s.setTimestamp(clock.getElapsedTime().asMilliseconds());
                                    }
                                    buffer.updateNextPlayerState(p, s);
                                }
                            }

                            semaphore.acquire();
                            State s = State(time, position, inputs, radius,
                                attack, player.getWpn().getId(), playerState.getPoint());
                            s.setAttackTimestamp(playerState.getAttackTimestamp());

                            // Keeps track of last attack timestamp, to make sure we can't spam attack and be lucky on tick timing:
                            if (inputs.getAttack() and clock.getElapsedTime().asMilliseconds() - playerState.getAttackTimestamp() >= (player.getWpn().getAttackSpeed() + player.getWpn().getReload())) {
                                s.setAttackTimestamp(clock.getElapsedTime().asMilliseconds());
                            }
                            buffer.updateNextPlayerState(player, s);
                            semaphore.release();
                        }
                        break;
                    }
//                    case Pkt::END_GAME: {
//                        int tick;
//                        short clientPort;
//
//                        packet >> tick >> clientPort;
//
//                        std::cout << "END GAME received" << std::endl;
//                        loop = false;
//                        break;
//                    }

                    case Pkt::END_R: {
                        // TODO: Handle end round packet.
                        break;
                    }
                    default: {
                        std::cout << "UNKNOWN PACKET TO SEND! Type: " << type << std::endl;
                        break;
                    }
                }
            }

            console.addPacket(id, type, COMM_PORT_SERVER, clock.getElapsedTime().asMilliseconds(), true);
        }
    }
}

/**
 * Loop that executes every tick rate: Needed info will be sent to the clients whenever needed. This is in a while loop.
 *
 *  @return int code for info as to how the server ended (type Err::)
 */
[[noreturn]] void Server::sendLoop() {
    std::optional<sf::IpAddress> sender = sf::IpAddress::resolve("127.0.0.1");
    sf::Packet packet;
    short type;
    uint32_t id;

    while (true) {
        while (!loop) {sf::sleep(sf::Time());} // Pause if needed
        sf::sleep(std::chrono::milliseconds(1000 / tickrate));

        std::unordered_map<std::string, State> currentState = buffer.getCurrentState();

        int tick = clock.getElapsedTime().asMilliseconds();

        // Check if a player won:
        unsigned short playersDead = 0;
        for (auto &[name, player] : clients) {
            playersDead += player.getStatus() == Status::DEAD;
        }

        if (playersDead+1 == clients.size()) {
            mapID = -1;     // put a mapID back to "not set"
            for (auto &[name, player] : clients) {
                if (player.getStatus() != Status::DEAD) {
                    clients[name].setStatus(Status::WIN);
                }
            }
        }

        for (auto & [port, player] : clients) {
            packet.clear();
            semaphore.acquire();
            id = getPacketId();
            packet << id;
            semaphore.release();
            switch (player.getStatus()) {
                // If user has been created but didn't receive confirmation yet:
                case Status::WAITING_FOR_INIT: {
                    type = Pkt::ACK;
                    packet << Pkt::ACK << Pkt::NEW_PLAYER;
                    break;
                }

                // If user has been ACKd, and is waiting to know their opponents:
                case Status::WAITING_FOR_OPPONENTS: {
                    bool ready = false;
                    // Check if everyone is ready first:
                    if (clients.size() == maxPlayers) {
                        // Map
                        if(mapID==-1) {
                            mapID = tick%Const::MAP_LINK.size();
                        }

                        ready = true;
                        type = Pkt::READY_R;
                        packet << Pkt::READY_R << mapID;
                        int nb = 1;
                        for (auto &[n, p] : clients) {
                            // Check that everyone know they've been ACKd by server:
                            ready = ready and (p.getStatus() == Status::WAITING_FOR_OPPONENTS or p.getStatus() == Status::READY_TO_START);
                            // Set position of clients:
                            Position pos;
                            pos.setX(Const::MAP_SIZE_X / 2 + (Const::MAP_SIZE_X / 4) * std::cos(nb*(2*M_PI / clients.size())));
                            pos.setY(Const::MAP_SIZE_Y / 2 + (Const::MAP_SIZE_Y / 4) * std::sin(nb*(2*M_PI / clients.size())));

                            // Set radius of clients:
                            float centerX = Const::MAP_SIZE_X / 2.0f;
                            float centerY = Const::MAP_SIZE_Y / 2.0f;

                            float dx = centerX - pos.getX();
                            float dy = centerY - pos.getY();

                            float angleToCenter = std::atan2(dy, dx);

                            if(p.getWpn().getType() == Weapons::RECTANGLE) {
                                angleToCenter = angleToCenter - p.getWpn().getRange()/2.f;
                            }

                            Input inputs;

                            State s(tick, pos, inputs, angleToCenter, false, 0, 100);
                            buffer.updateNextPlayerState(p, s);

                            // Add everything in packet:
                            packet << p.getName() << p.getColor().r << p.getColor().g << p.getColor().b << p.getColor().a;
                            packet << p.getWeapons()[1];
                            nb++;
                        }
                    }
                    else {
                        std::cout << "Received " << clients.size() << "/" << maxPlayers << " players." << std::endl;
                    }

                    // If not ready, send an ACK that client is waiting:
                    if (!ready) {
                        packet.clear();
                        type = Pkt::ACK;
                        packet << id << Pkt::ACK << Pkt::WAIT_OPPONENTS;
                    }
                    break;
                }

                // User has been ACKd by server, and user also has opponents added:
                case Status::READY_TO_START: {
                    // Check if everyone is ready to start:
                    bool ready = true;
                    for (auto &[n, p] : clients) {
                        ready = ready and p.getStatus() == Status::READY_TO_START;
                    }

                    if (ready) {
                        type = Pkt::GLOBAL;
                        packet << Pkt::GLOBAL << int(buffer.getCurrentTick());

                        for (auto & [n, state] : currentState) {
                            packet << n << state;
                        }
                    }
                    else {
                        type = Pkt::ACK;
                        packet << Pkt::ACK << Pkt::WAIT_START_R;
                    }
                    break;
                }

                case Status::DONE: {
                    type = Pkt::GLOBAL;
                    packet << Pkt::GLOBAL << int(buffer.getCurrentTick()) << clock.getElapsedTime().asMilliseconds();
                    for (auto & [n, state] : currentState) {
                        packet << n << state;
                    }

                    semaphore.acquire();
                    addLine(
                    "Server >>> " + clients[port].getName()
                    + " position: (" + std::to_string(currentState[clients[port].getName()].getPosition().getY())
                    + ", " + std::to_string(currentState[clients[port].getName()].getPosition().getY())
                    + ") ; radius : " + std::to_string(currentState[clients[port].getName()].getRadius())
                    + " attack : " + std::to_string(currentState[clients[port].getName()].getAttack())
                    , sf::Color::White);
                    addToGraph(clock.getElapsedTime().asMilliseconds(), "Server", "clients");
                    semaphore.release();
                    break;
                }

                case Status::DEAD: {
                    type = Pkt::DEATH;
                    packet << Pkt::DEATH;
                    break;
                }

                case Status::WIN: {
                    type = Pkt::WIN;
                    packet << Pkt::WIN;
                }
            }
            semaphore.acquire();
            socket.send(packet, sender.value(), player.getPort());
            console.addPacket(id, type, COMM_PORT_SERVER, clock.getElapsedTime().asMilliseconds());
            semaphore.release();
        }
        semaphore.acquire();
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
    }
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
            socket.send(packet, sender.value(), player.getPort());
        }
        return Err::ERR_NONE;
    }

    loop = false;

    return Err::ERR_SERVER_SHUTDOWN;
}