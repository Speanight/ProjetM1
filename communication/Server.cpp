#include "Server.hpp"

/**
 * A server is being initialized with MainWindow. It's a needed component to ensure communication between the different
 * clients. It receives the packets, and send packets back to each client. For that, they need to be added manually
 * after creation of the server.
 *
 * @param clock Clock, needed to synchronise clients and server together for packet transmission.
 */
Server::Server(const sf::Clock clock) : semaphore(1) {
    colors = {sf::Color::White, sf::Color::Red, sf::Color::Cyan, sf::Color::Green, sf::Color::Yellow};
    this->clock = clock;
    if (socket.bind(COMM_PORT_SERVER) != sf::Socket::Status::Done) {
        std::cout << "Error: port isn't available?" << std::endl;
    }
    else {
        socket.setBlocking(true);
        sendThread = std::thread(&Server::sendLoop, this);
        receiveThread = std::thread(&Server::receiveLoop, this);

        // Adds all compensation methods:
        compensations[Compensation::EXTRAPOLATION] = true;
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
    Player player(
        std::any_cast<unsigned short>(infos["port"]),
        std::any_cast<std::string>(infos["name"]),
        std::any_cast<sf::Color>(infos["color"]),
        Position()
    );
    clients[std::any_cast<std::string>(infos["name"])] = player;
    buffer.addClient(player);
    std::cout << "Added client " << std::any_cast<std::string>(infos["name"]) << " on port " << clients[std::any_cast<std::string>(infos["name"])].port << std::endl;
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
    int senderNum;
    const sf::Time tickrate = std::chrono::milliseconds(TICKRATE);
    int dt;
    std::unordered_map<std::string, State> currentState;
    State playerState;
    float radius;

    while (loop) {
        sf::sleep(sf::Time());
        senderNum = 0;

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
                }
            }

            // Checks for all connected clients:
            for (auto &[name, player]: clients) {
                senderNum++;
                if (player.port == port) { // Check if ports corresponds (AKA the expected client)
                    // printf("port OK\n");
                    packet >> type;

                    switch (type) {
                        case Pkt::ROUND_START: {
                            newGame = true; // Some players aren't ready for round start!
                        }

                        case Pkt::ACK: {
                            int context;
                            packet >> context;

                            switch (context) {
                                case Pkt::ROUND_START:
                                    newGame = false;
                                    break;
                                default:
                                    std::cout << "Received unknown ACK: " << context << " from client " << name << std::endl;
                            }

                            break;
                        }
                        case Pkt::INPUTS: {
                            int time;
                            packet >> time >> inputs;

                            semaphore.acquire();
                            addLine(
                                name + " >>> Server [PING:" + std::to_string(clock.getElapsedTime().asMilliseconds() - time) +"ms] "
                                +" | inputs: x=" + std::to_string(inputs.getMovementX()) +
                                "; y=" + std::to_string(inputs.getMovementY()) +
                                "; rotate = "+ std::to_string(inputs.getRotate()) +
                                "; mode = " + std::to_string(inputs.getMode())
                                );
                            semaphore.release();
                            currentState = buffer.getCurrentState();
                            playerState = buffer.getLastState(player);
                            // ====== POSITION ======
                            position = playerState.getPosition();
                            dt = (time - playerState.getTimestamp()) % (2 * Const::TICKRATE.count());
                            position.setX(position.getX() + inputs.getMovementX() * Const::PLAYER_SPEED * dt);
                            position.setY(position.getY() + inputs.getMovementY() * Const::PLAYER_SPEED * dt);

                            // ====== WEAPON RADIUS ======
                            radius = buffer.getCurrentState()[name].getRadius();           // give the actual radius of the client (weapon position)
                            //Check if the radius is not too much
                            if (radius + inputs.getRotate() > 360.f) { //TODO : seems to bug a bit
                                radius = radius + inputs.getRotate() - 360.f;
                            } else {
                                if (radius + inputs.getRotate() < 0) {
                                    radius = 360.f + radius + inputs.getRotate();

                            float twoPi = 2.f * std::numbers::pi;

                            if (radius >= twoPi)
                                radius -= twoPi;
                            else if (radius < 0.f)
                                radius += twoPi;

                            // ====== WEAPON MODE ======
                            bool mode;
                            if(inputs.getMode()) {
                                // printf("CLICK 2 !\n");
                                mode = !buffer.getCurrentState()[name].getMode();
                            }
                            else {
                                mode = buffer.getCurrentState()[name].getMode();
                            }
                            buffer.getCurrentState()[name].setMode(mode);

                            for (auto &[n, p]: clients) {
                                if (name != n) {
                                    Position opponentPos = currentState[n].getPosition();
                                    Position pos = resolveCollision(position, opponentPos);
                                    if (pos.getX() != currentState[n].getPosition().getX() and
                                        pos.getY() != currentState[n].getPosition().getY()) {
                                        State s = State(time, pos, inputs);
                                        buffer.refreshBuffer(p, s, time);

                                    float opponentRadius = buffer.getCurrentState()[n].getRadius();
                                    bool opponentMode = buffer.getCurrentState()[n].getMode();

                                    if (pos.getX() != buffer.getCurrentState()[n].getPosition().getX() and pos.getY() != buffer.getCurrentState()[n].getPosition().getY()) {
                                        State s = State(time, pos, opponentRadius, opponentMode, inputs);
                                        buffer.refreshBuffer(p, s, time);
                                    }
                                }

                                semaphore.acquire();
                                State s = State(time, position, radius, mode, inputs);
                                buffer.refreshBuffer(player, s, time);
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

    while (loop) {
        // DEFAULT BEHAVIOR - SEND SERVER-SIDE POSITIONS AND RADIUS
        std::unordered_map<std::string, State> currentState = buffer.getCurrentState();
        for (auto & [name, player] : clients) {
            packet.clear();

            if (newGame) {
                packet << Pkt::ROUND_START << int(buffer.getCurrentTick()) << int(clients.size());

                int playerNb = 1;
                int time = clock.getElapsedTime().asMilliseconds();

                for (auto & [name, player] : clients) {
                    Position pos = buffer.getCurrentState()[name].getPosition();
                    pos.setX((playerNb * Const::MAP_SIZE_X / (clients.size())) - (Const::MAP_SIZE_X / clients.size()) / 2);
                    pos.setY(Const::MAP_SIZE_Y / 2);
                    Input inputs(0, 0, 0.f, false, false);
                    State s = State(time, pos, std::numbers::pi/2, true, inputs);
                    buffer.refreshBuffer(player, s, time);
                    buffer.setPlayerPosition(name, pos);

                    float radius = buffer.getCurrentState()[name].getRadius();           // give the actual radius of the client (weapon position)
                    buffer.refreshBuffer(player, s, time);
                    buffer.getCurrentState()[name].getRadius();
                    playerNb++;
                }
            }
            else {
                packet << Pkt::GLOBAL << int(buffer.getCurrentTick()) << int(currentState.size());
            }

            for (auto & [n, state] : currentState) {
                packet << n << state.getPosition() << state.getRadius() << state.getRadius() << state.getMode();
            }
            semaphore.acquire();
            socket.send(packet, sender.value(), player.port);
            addLine(
                "Server >>> " + name
                + " position: (" + std::to_string(player.position.getX())
                + ", " + std::to_string(player.position.getY())
                + ") ; radius : " + std::to_string(player.radius)
                + " mode : " + std::to_string(player.mode)
                , sf::Color::White);
            semaphore.release();
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
