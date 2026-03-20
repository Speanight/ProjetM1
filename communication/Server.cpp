#include "Server.hpp"

/**
 * A server is being initialized with MainWindow. It's a needed component to ensure communication between the different
 * clients. It receives the packets, and send packets back to each client. For that, they need to be added manually
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
 * Allows to add clients to the server's tracked routes. This means the server will send and receive packets from the
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

    clients[std::any_cast<std::string>(infos["name"])] = player;
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
    int senderNum;
    const sf::Time tickrate = std::chrono::milliseconds(TICKRATE);
    int dt;
    std::unordered_map<std::string, State> currentState;
    State playerState;
    float radius;

    while (loop) {
        sf::sleep(sf::Time());
        senderNum = 0;
        packet.clear();

        if (socket.receive(packet, sender, port) == sf::Socket::Status::Done) {
            // Checks for all connected clients:
            // std::cout<<"SERVER RECEIVE A PACKET"<<std::endl;
            packet >> type;
            if(type == Pkt::NEW_PLAYER) {
                // tick << name << r g b a << wpn << port
                int tick;
                std::string pname;
                short r, g, b, a;
                int wpn_id;
                short clientPort;

                packet >> tick >> pname >> r >> g >> b >> a >> wpn_id >> clientPort;

                sf::Color color(r, g, b, a);

                // ===== CHECK IF PLAYER ALREADY EXISTS =====
                semaphore.acquire();
                if (clients.contains(pname)) {
                    // std::cout << "player already exist: " << pname << std::endl;
                }
                else {
                    std::cout << "NEW PLAYER: " << pname << std::endl;

                    // ===== CREATE PLAYER =====
                    Player newPlayer;
                    newPlayer.name = pname;
                    newPlayer.color = color;
                    newPlayer.port = clientPort;
                    newPlayer.position = Position(0, 0);
                    newPlayer.radius = 0.f;
                    newPlayer.mode = true;
                    newPlayer.isAttacking = false;

                    Weapon wpn(wpn_id);
                    newPlayer.wpn = wpn;

                    // ===== ADD TO CLIENTS =====
                    clients[pname] = newPlayer;

                    // ===== ADD TO BUFFER =====
                    buffer.addClient(newPlayer);

                    // Création d’un state initial
                    int time = clock.getElapsedTime().asMilliseconds();

                    Input defaultInput(0, 0.f, 0.f, 0.f, false, false, wpn_id);

                    State initialState(time,
                        Position(0, 0),
                        0.f,
                        true,
                        false,
                        wpn_id,
                        100,
                        defaultInput
                        );

                    buffer.updateNextPlayerState(newPlayer, initialState);
                    buffer.push(time);

                    std::cout << "Player " << pname << " added to server." << std::endl;
                }

                // ===== PREPARE ACK =====
                this->packetTypeToSend = Pkt::ACK;
                this->ackToSend = Pkt::NEW_PLAYER;
                semaphore.release();
            }
            else {
                for (auto &[name, player]: clients) {
                    senderNum++;
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
                                        std::cout << "THE WAR MAY BEGIN ..." << std::endl;
                                        semaphore.acquire();
                                        packetTypeToSend = Pkt::START_R;
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
                                    +" | inputs: x=" + std::to_string(inputs.getMovementX()) +
                                    "; y=" + std::to_string(inputs.getMovementY()) +
                                    "; rotate = "+ std::to_string(inputs.getRotate()) +
                                    "; mode = " + std::to_string(inputs.getMode()) +
                                    "; attack = " + std::to_string(inputs.getAttack()) +
                                    "; wpn id = " + std::to_string(inputs.getWpnID()) +
                                    "; inputs #" + std::to_string(inputs.getId()),
                                    player.color
                                    );
                                addToGraph(clock.getElapsedTime().asMilliseconds(), name, "Server");
                                semaphore.release();

                                // Get the current server state AND last player state (which might be the next server state!)
                                currentState = buffer.getCurrentState();
                                semaphore.acquire();
                                playerState = buffer.getLastState(player);
                                semaphore.release();
                                buffer.addInputsToLastState(player, clock.getElapsedTime().asMilliseconds(), inputs);

                                // ====== POSITION ======
                                position = playerState.getPosition();
                                semaphore.acquire();
                                radius = buffer.getLastState(player).getRadius();
                                semaphore.release();

                                // Get time elapsed since last packet from client. Used for consistency in speed and such.
                                dt = (time - playerState.getTimestamp()) % (Const::TICKRATE.count());

                                // Adjust client values according to last state and new inputs values.
                                position.move(inputs.getMovementX(), inputs.getMovementY(), dt);
                                float twoPi = 2.f * std::numbers::pi;
                                radius += std::fmod(inputs.getRotate() * Const::PLAYER_RADIUS_SPEED * dt, twoPi);

                                // ====== WEAPON MODE ======
                                bool mode = playerState.getMode();
                                if(inputs.getMode()) {
                                    mode = !mode;
                                }

                                // ====== ATTACK ======
                                bool attack = inputs.getAttack() or playerState.getAttack();
                                if(attack)("input => %d, playerstate => %d\n", inputs.getAttack(), playerState.getAttack());

                                // ====== WEAPON DATAS CHANGE ======
                                int wpn_id = playerState.getWpn().getId();
                                currentState[name].setWpn(wpn_id);

                                // loops of all interaction between players
                                for (auto &[n, p]: clients) {
                                    if (name != n) {
                                        bool interaction = false;
                                        // Check if there is a collision between players (and therefor if it should be resolved)
                                        Position opponentPos = currentState[n].getPosition();
                                        opponentPos = resolveCollision(position, opponentPos);

                                        // If yes, we re-adjust the new position of said opponent:
                                        if (opponentPos.getX() != currentState[n].getPosition().getX() and
                                            opponentPos.getY() != currentState[n].getPosition().getY()) {
                                            interaction = true;
                                            }


                                        if(inputs.getAttack()) {
                                            interaction = true;
                                            // ====== POINT GESTION ======
                                            Position opponentPos = currentState[n].getPosition();

                                            float dx = opponentPos.getX() - position.getX();
                                            float dy = opponentPos.getY() - position.getY();

                                            float distAB = std::sqrt(dx*dx + dy*dy);

                                            // max distance for the weapon to touch the enemy
                                            float maxReach = PLAYER_SIZE * 2.f
                                                + currentState[name].getWpn().getHeight()
                                                + currentState[name].getWpn().getRange();

                                            if (distAB <= maxReach){    // if players close enough for the weapon to touch
                                                // weapopn direction
                                                float dirx = std::cos(radius);
                                                float diry = std::sin(radius);

                                                // distance between the player and the surface of the opponent
                                                float attackReach = maxReach - PLAYER_SIZE;

                                                float topx = position.getX() + dirx * attackReach;
                                                float topy = position.getY() + diry * attackReach;

                                                float dx2 = opponentPos.getX() - topx;
                                                float dy2 = opponentPos.getY() - topy;

                                                float distTop = std::sqrt(dx2*dx2 + dy2*dy2);

                                                if (distTop <= PLAYER_SIZE){        // if the weapon can enter the opponent perimeters, then it's a touch
                                                    bool blocked = false;

                                                    bool opponentMode = currentState[n].getMode();
                                                    float opponentRadius = currentState[n].getRadius();

                                                    if (!opponentMode) {  // if the opponent have it's defense activated

                                                        // looking for the angle between the player and it's opponent
                                                        float angleToAttacker = std::atan2(
                                                            position.getY() - opponentPos.getY(),
                                                            position.getX() - opponentPos.getX()
                                                        );

                                                        float twoPi = 2.f * std::numbers::pi;

                                                        auto normalize = [&](float a) {
                                                            a = std::fmod(a, twoPi);
                                                            if (a < 0) a += twoPi;
                                                            return a;
                                                        };

                                                        angleToAttacker = normalize(angleToAttacker);
                                                        opponentRadius  = normalize(opponentRadius);

                                                        float shieldStart = normalize(opponentRadius - 0.8f);
                                                        float shieldEnd   = normalize(opponentRadius + 0.8f);

                                                        if (shieldStart < shieldEnd)
                                                            blocked = (angleToAttacker >= shieldStart && angleToAttacker <= shieldEnd);
                                                        else
                                                            blocked = (angleToAttacker >= shieldStart || angleToAttacker <= shieldEnd);
                                                    }

                                                    if (!blocked) {
                                                        // HIT SECTION

                                                        int pts =  currentState[n].getPoint() - playerState.getWpn().getDamage();
                                                        if (pts <= 0) {
                                                            std::cout << name << " kill  " << n <<std::endl;
                                                            currentState[n].setKillerName(name);
                                                            currentState[n].setPoint(0);
                                                        }
                                                        else {
                                                            currentState[n].setPoint(pts);
                                                        }
                                                    }
                                                    else {
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
                                        }

                                        if(interaction) {
                                            State s = State(time,
                                                opponentPos,
                                                currentState[n].getRadius(),
                                                currentState[n].getMode(),
                                                currentState[n].getAttack(),
                                                currentState[n].getWpn().getId(),
                                                currentState[n].getPoint(),
                                                inputs);

                                            s.setKillerName(currentState[n].getKillerName());

                                            // TODO : change constructor ?

                                            buffer.updateNextPlayerState(p, s);
                                        }
                                    }

                                    semaphore.acquire();
                                    State s = State(
                                        time,
                                        position,
                                        radius,
                                        mode,
                                        attack,
                                        wpn_id,
                                        playerState.getPoint(),
                                        inputs
                                    );

                                    buffer.updateNextPlayerState(player, s, playerState.getMode());
                                    semaphore.release();

                                }
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

                        /*
                        // switch (type) {
                        //     case Pkt::NEW_GAME: {
                        //         printf("NEW_GAME\n");
                        //     }
                        //     case Pkt::ROUND_START: {
                        //         partyState = PartyState::RUN_START;
                        //         newRound = true; // Some players aren't ready for round start!
                        //     }
                        //     case Pkt::ACK: {
                        //         int context;
                        //         packet >> context;
                        //
                        //         switch (context) {
                        //             case Pkt::ROUND_START:
                        //                 partyState = PartyState::RUN_START;
                        //                 newRound = false;
                        //                 break;
                        //             default:
                        //                 std::cout << "Received unknown ACK: " << context << " from client " << name << std::endl;
                        //         }
                        //         break;
                        //     }
                        //     case Pkt::INPUTS: {
                        //         int time;
                        //         packet >> time >> inputs;
                        //
                        //         // Get threads priority
                        //         semaphore.acquire();
                        //         addLine(
                        //             name + " >>> Server [PING:" + std::to_string(clock.getElapsedTime().asMilliseconds() - time) +"ms] "
                        //             +" | inputs: x=" + std::to_string(inputs.getMovementX()) +
                        //             "; y=" + std::to_string(inputs.getMovementY()) +
                        //             "; rotate = "+ std::to_string(inputs.getRotate()) +
                        //             "; mode = " + std::to_string(inputs.getMode()) +
                        //             "; attack = " + std::to_string(inputs.getAttack()) +
                        //             "; wpn id = " + std::to_string(inputs.getWpnID()) +
                        //             "; inputs #" + std::to_string(inputs.getId()),
                        //             player.color
                        //             );
                        //         addToGraph(clock.getElapsedTime().asMilliseconds(), name, "Server");
                        //         semaphore.release();
                        //
                        //         // Get the current server state AND last player state (which might be the next server state!)
                        //         currentState = buffer.getCurrentState();
                        //         semaphore.acquire();
                        //         playerState = buffer.getLastState(player);
                        //         semaphore.release();
                        //         buffer.addInputsToLastState(player, clock.getElapsedTime().asMilliseconds(), inputs);
                        //
                        //         // ====== POSITION ======
                        //         position = playerState.getPosition();
                        //         semaphore.acquire();
                        //         radius = buffer.getLastState(player).getRadius();
                        //         semaphore.release();
                        //
                        //         // Get time elapsed since last packet from client. Used for consistency in speed and such.
                        //         dt = (time - playerState.getTimestamp()) % (Const::TICKRATE.count());
                        //
                        //         // Adjust client values according to last state and new inputs values.
                        //         position.move(inputs.getMovementX(), inputs.getMovementY(), dt);
                        //         float twoPi = 2.f * std::numbers::pi;
                        //         radius += std::fmod(inputs.getRotate() * Const::PLAYER_RADIUS_SPEED * dt, twoPi);
                        //
                        //         // ====== WEAPON MODE ======
                        //         bool mode = playerState.getMode();
                        //         if(inputs.getMode()) {
                        //             mode = !mode;
                        //         }
                        //
                        //         // ====== ATTACK ======
                        //         bool attack = inputs.getAttack() or playerState.getAttack();
                        //         if(attack)("input => %d, playerstate => %d\n", inputs.getAttack(), playerState.getAttack());
                        //
                        //         // ====== WEAPON DATAS CHANGE ======
                        //         int wpn_id = playerState.getWpn().getId();
                        //         currentState[name].setWpn(wpn_id);
                        //
                        //         // loops of all interaction between players
                        //         for (auto &[n, p]: clients) {
                        //             if (name != n) {
                        //                 bool interaction = false;
                        //                 // Check if there is a collision between players (and therefor if it should be resolved)
                        //                 Position opponentPos = currentState[n].getPosition();
                        //                 opponentPos = resolveCollision(position, opponentPos);
                        //
                        //                 // If yes, we re-adjust the new position of said opponent:
                        //                 if (opponentPos.getX() != currentState[n].getPosition().getX() and
                        //                     opponentPos.getY() != currentState[n].getPosition().getY()) {
                        //                     interaction = true;
                        //                 }
                        //
                        //
                        //                 if(inputs.getAttack()) {
                        //                     interaction = true;
                        //                     // ====== POINT GESTION ======
                        //                     Position opponentPos = currentState[n].getPosition();
                        //
                        //                     float dx = opponentPos.getX() - position.getX();
                        //                     float dy = opponentPos.getY() - position.getY();
                        //
                        //                     float distAB = std::sqrt(dx*dx + dy*dy);
                        //
                        //                     // max distance for the weapon to touch the enemy
                        //                     float maxReach = PLAYER_SIZE * 2.f
                        //                         + currentState[name].getWpn().getHeight()
                        //                         + currentState[name].getWpn().getRange();
                        //
                        //                     if (distAB <= maxReach){    // if players close enough for the weapon to touch
                        //                         // weapopn direction
                        //                         float dirx = std::cos(radius);
                        //                         float diry = std::sin(radius);
                        //
                        //                         // distance between the player and the surface of the opponent
                        //                         float attackReach = maxReach - PLAYER_SIZE;
                        //
                        //                         float topx = position.getX() + dirx * attackReach;
                        //                         float topy = position.getY() + diry * attackReach;
                        //
                        //                         float dx2 = opponentPos.getX() - topx;
                        //                         float dy2 = opponentPos.getY() - topy;
                        //
                        //                         float distTop = std::sqrt(dx2*dx2 + dy2*dy2);
                        //
                        //                         if (distTop <= PLAYER_SIZE){        // if the weapon can enter the opponent perimeters, then it's a touch
                        //                             bool blocked = false;
                        //
                        //                             bool opponentMode = currentState[n].getMode();
                        //                             float opponentRadius = currentState[n].getRadius();
                        //
                        //                             if (!opponentMode) {  // if the opponent have it's defense activated
                        //
                        //                                 // looking for the angle between the player and it's opponent
                        //                                 float angleToAttacker = std::atan2(
                        //                                     position.getY() - opponentPos.getY(),
                        //                                     position.getX() - opponentPos.getX()
                        //                                 );
                        //
                        //                                 float twoPi = 2.f * std::numbers::pi;
                        //
                        //                                 auto normalize = [&](float a) {
                        //                                     a = std::fmod(a, twoPi);
                        //                                     if (a < 0) a += twoPi;
                        //                                     return a;
                        //                                 };
                        //
                        //                                 angleToAttacker = normalize(angleToAttacker);
                        //                                 opponentRadius  = normalize(opponentRadius);
                        //
                        //                                 float shieldStart = normalize(opponentRadius - 0.8f);
                        //                                 float shieldEnd   = normalize(opponentRadius + 0.8f);
                        //
                        //                                 if (shieldStart < shieldEnd)
                        //                                     blocked = (angleToAttacker >= shieldStart && angleToAttacker <= shieldEnd);
                        //                                 else
                        //                                     blocked = (angleToAttacker >= shieldStart || angleToAttacker <= shieldEnd);
                        //                             }
                        //
                        //                             if (!blocked) {
                        //                                 // HIT SECTION
                        //
                        //                                 int pts =  currentState[n].getPoint() - playerState.getWpn().getDamage();
                        //                                 if (pts <= 0) {
                        //                                     std::cout << name << " kill  " << n <<std::endl;
                        //                                     currentState[n].setKillerName(name);
                        //                                     currentState[n].setPoint(0);
                        //                                 }
                        //                                 else {
                        //                                     currentState[n].setPoint(pts);
                        //                                 }
                        //                             }
                        //                             else {
                        //                                 // BLOCKED SECTION
                        //                                 float dirx = std::cos(radius);
                        //                                 float diry = std::sin(radius);
                        //
                        //                                 // distance de knockback = 2x la range de l'arme
                        //                                 float knockbackDist = currentState[name].getWpn().getRange() * 2.f;
                        //
                        //                                 // nouvelle position du joueur attaquant
                        //                                 position.setX(position.getX() - dirx * knockbackDist);
                        //                                 position.setY(position.getY() - diry * knockbackDist);
                        //                             }
                        //                         }
                        //                     }
                        //                 }
                        //
                        //                 if(interaction) {
                        //                     State s = State(time,
                        //                         opponentPos,
                        //                         currentState[n].getRadius(),
                        //                         currentState[n].getMode(),
                        //                         currentState[n].getAttack(),
                        //                         currentState[n].getWpn().getId(),
                        //                         currentState[n].getPoint(),
                        //                         inputs);
                        //
                        //                     s.setKillerName(currentState[n].getKillerName());
                        //
                        //                     // TODO : change constructor ?
                        //
                        //                     buffer.updateNextPlayerState(p, s);
                        //                 }
                        //             }
                        //
                        //             semaphore.acquire();
                        //             State s = State(
                        //                 time,
                        //                 position,
                        //                 radius,
                        //                 mode,
                        //                 attack,
                        //                 wpn_id,
                        //                 playerState.getPoint(),
                        //                 inputs
                        //             );
                        //
                        //             buffer.updateNextPlayerState(player, s, playerState.getMode());
                        //             semaphore.release();
                        //
                        //         }
                        //         break;
                        //     }
                        //     case Pkt::END_GAME: {
                        //         printf("ENDGAME\n");
                        //         // endGame = true;
                        //     }
                        //
                        //     default: {
                        //         std::cout << " UNKNOWN CLIENT PACKET! Type: " << type << " from client " << name << std::endl;
                        //     }
                        // }
                        */
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

    while (loop) {
        sf::sleep(tickrate);

        std::unordered_map<std::string, State> currentState = buffer.getCurrentState();
        int tick = clock.getElapsedTime().asMilliseconds();

        for (auto & [name, player] : clients) {
            sf::Packet packet;
            /*
            //            switch (player.status) {
            //                // If player is still waiting for round start packet:
            //                case Status::WAITING_FOR_ROUND_START: {
            //                    player.status = Status::WAITING_FOR_ROUND_START;
            //                    packet << Pkt::ROUND_START << int(buffer.getCurrentTick()) << int(clients.size());
            //
            //                    int playerNb = clients.size();
            //                    int time = clock.getElapsedTime().asMilliseconds();
            //
            //                    for (auto & [name, player] : clients) {
            //                        Position pos;
            //                        pos.setX((playerNb * Const::MAP_SIZE_X / (clients.size())) - (Const::MAP_SIZE_X / clients.size()) / 2);
            //                        pos.setY(Const::MAP_SIZE_Y / 2);
            //                        Input inputs(buffer.getLastState(player).getInputs().end()->second.getId(), 0, 0, 0.f, false, false, 0);
            //                        State s = State(time, pos, std::numbers::pi/2, true, inputs);
            //                        buffer.updateNextPlayerState(player, s);
            //
            //                        playerNb--;
            //                    }
            //                    buffer.push(clock.getElapsedTime().asMilliseconds());
            //                    currentState = buffer.getCurrentState(); // Refresh current state.
            //                    break;
            //                }
            //
            //                case Status::READY_TO_START:
            //                    packet << Pkt::ACK << Pkt::ROUND_START << int(buffer.getCurrentTick()) << int(clients.size());
            //                    break;
            //
            //                case Status::DONE:
            //
            //            }
            */
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
                            std::cout << "Unrecognized acknoledge packet sent in server section, please identify yourself " << ackToSend << std::endl;
                            break;
                        }
                    }
                    break;
                }
                case Pkt::READY_R       : {     // tick << client infos
                    std::cout << "sending position" << std::endl;
                    packet << Pkt::READY_R;
                    packet << tick << maxPlayers;

                    int playerNb = maxPlayers;

                    for (auto & [n, p] : clients) {
                        Position pos;
                        pos.setX((playerNb * Const::MAP_SIZE_X / maxPlayers) - (Const::MAP_SIZE_X / maxPlayers) / 2);
                        pos.setY(Const::MAP_SIZE_Y / 2);

                        Input inputs(buffer.getLastState(p).getInputs().end()->second.getId(), 0, 0, 0.f, false, false, 0);
                        State s = State(tick, pos, std::numbers::pi/2, true, false, 0, 100, inputs);

                        buffer.updateNextPlayerState(p, s);

                        packet << n << s << p.color.r << p.color.g << p.color.b << p.color.a << p.wpn.getId();
                        playerNb--;
                    }

                    buffer.push(tick);
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
                    packet << int(clients.size());

                    for (auto & [n, state] : currentState) {
                        packet << n << state;
                    }

                    packet << tick; // lastServerTick
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
                            State s = State(time, pos, std::numbers::pi/2, true, false, 0, 100, inputs);
                            buffer.updateNextPlayerState(player, s);

                            playerNb--;
                        }
                        buffer.push(clock.getElapsedTime().asMilliseconds());
                        currentState = buffer.getCurrentState(); // Refresh current state.
                    }
                    case PartyState::RUN_IDLE : {
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
                            + " mode : " + std::to_string(player.mode)
                            + " attack : " + std::to_string(player.isAttacking)
                            , sf::Color::White);
                        semaphore.release();
                    }
                }
            }

            semaphore.acquire();
            addToGraph(clock.getElapsedTime().asMilliseconds(), "Server", "clients");
            buffer.push(clock.getElapsedTime().asMilliseconds());
            semaphore.release();

            // POST MAJ
            for (auto & [name, player] : clients) {
                State last = buffer.getLastState(player);
                if(last.getAttack()) {          // setting the attack save into false one so we don't keep the attack signal
                    // printf("putting last set into false\n");
                    last.setAttack(false);
                    buffer.updateNextPlayerState(player, last);
                }
            }


            sf::sleep(tickrate);
        }
        */

            if(packetTypeToSend != Pkt::NONE) {
                // SENDING TO CLIENT
                socket.send(packet, sender.value(), player.port);

                // PRINT MESSAGE IN SERVER CONSOLE
                addLine(
                    "Server >>> " + name
                    + " position: (" + std::to_string(player.position.getX())
                    + ", " + std::to_string(player.position.getY())
                    + ") ; radius : " + std::to_string(player.radius)
                    + " mode : " + std::to_string(player.mode)
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
            }
            else {
                std::cout << "waiting for players" << std::endl;
            }
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
