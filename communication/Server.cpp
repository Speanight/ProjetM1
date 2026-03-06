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
            for (auto & [name, player] : clients) {
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
                            printf("%f\n", inputs.getMovementX());
                            printf("%f\n", inputs.getRotate());
                            addLine(
                                name + " >>> Server [PING:" + std::to_string(clock.getElapsedTime().asMilliseconds() - time) +"ms] "
                                +" | inputs: x=" + std::to_string(inputs.getMovementX()) +
                                "; y=" + std::to_string(inputs.getMovementY()) +
                                "; rotate = "+ std::to_string(inputs.getRotate())
                                );
                            semaphore.release();
                            Position position = buffer.currentState[name].getPosition();
                            int dt = (clock.getElapsedTime().asMilliseconds() - buffer.currentState[name].getTimestamp()) % 2*Const::TICKRATE.count();
                            position.setX(position.getX() + inputs.getMovementX() * Const::PLAYER_SPEED * dt / 1000);
                            position.setY(position.getY() + inputs.getMovementY() * Const::PLAYER_SPEED * dt / 1000);

                            // TODO : DELETE ME IF I'M NOT GOOD SORRY o7
                            float radius = buffer.currentState[name].getRadius();           // give the actual radius of the client (weapon position)
                            //Check if the radius is not too much
                            radius += inputs.getRotate();

                            // wrap angle
                            while (radius >= 360.f) radius -= 360.f;
                            while (radius < 0.f)    radius += 360.f;


                            for (auto & [n, player] : clients) {
                                if (name != n) {
                                    Position opponentPos = buffer.currentState[n].getPosition();
                                    Position pos = resolveCollision(position, opponentPos);

                                    // TODO : DELETE ME IF I'M NOT GOOD SORRY o7
                                    float oponentRadius = buffer.currentState[n].getRadius();

                                    // TODO : check if i have to add the radius in this ( @ ~ @)
                                    if (pos.getX() != buffer.currentState[n].getPosition().getX() and pos.getY() != buffer.currentState[n].getPosition().getY()) {
                                        State s = State(time, pos, radius,  inputs);
                                        refreshBuffer(n, s, time);
                                    }
                                }
                            }

                            semaphore.acquire();
                            State s = State(time, position, radius, inputs);
                            // player.position.setX(buffer.currentState[player.name].getPosition().getX());
                            // player.position.setY(buffer.currentState[player.name].getPosition().getY());
                            refreshBuffer(name, s, time);
                            semaphore.release();
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
        for (auto & [name, player] : clients) {
            packet.clear();

            if (newGame) {
                packet << Pkt::ROUND_START << int(buffer.stateTick) << int(clients.size());

                int playerNb = 1;
                int time = clock.getElapsedTime().asMilliseconds();

                for (auto & [name, player] : clients) {
                    Position pos = buffer.currentState[name].getPosition();
                    pos.setX((playerNb * Const::MAP_SIZE_X / (clients.size())) - (Const::MAP_SIZE_X / clients.size()) / 2);
                    pos.setY(Const::MAP_SIZE_Y / 2);

                    // TODO : DELETE ME IF I'M NOT GOOD SORRY o7
                    float radius = buffer.currentState[name].getRadius();           // give the actual radius of the client (weapon position)
                    // Input inputs(0,0,false);
                    Input inputs(0,0,0, false);
                    State s = State(time, pos, radius, inputs);
                    refreshBuffer(name, s, time);
                    buffer.currentState[name].setPosition(pos);
                    buffer.currentState[name].getRadius();
                    playerNb++;
                }
            }
            else {
                packet << Pkt::GLOBAL << int(buffer.stateTick) << int(buffer.currentState.size());
            }

            for (auto & [n, state] : buffer.currentState) {
                packet << n << state.getPosition() << state.getRadius();
            }
            semaphore.acquire();
            socket.send(packet, sender.value(), player.port);
            addLine("Server >>> " + name + " position: (" + std::to_string(player.position.getX()) + ", " + std::to_string(player.position.getY()) + ") and radius : " + std::to_string(player.radius), sf::Color::White);
            semaphore.release();
        }
        sf::sleep(tickrate);
    }


    receiveThread.join();
    return Err::ERR_NONE; // Exited without any issue.
}

void Server::refreshBuffer(const std::string& client, State state, int clockState) {
    buffer.nextState[client] = state;

    // We "push" the buffer if we're onto the next tick. (known by checking current server tick).
    if (buffer.stateTick / Const::TICKRATE.count() < clockState / Const::TICKRATE.count()) {
        for (auto & [name, remotePort] : clients) {

            // If some values are missing, we add them with potential compensations:
            if (auto search = buffer.nextState.find(name); search == buffer.nextState.end()) {
                buffer.nextState[name] = buffer.currentState[name]; // Rollbacks to previously known position.
                // Add compensations here:
//                int toCompensate = clockState - buffer.currentState[name].getTimestamp();
                // float toCompensate = 0.003; // TODO: find time to compensate!
                // if (compensations[Compensation::EXTRAPOLATION]) {
                //     // EXTRAPOLING - We guess the new position based of last known position AND inputs.
                //     Position position = buffer.nextState[name].getPosition();
                //     Input inputs = buffer.nextState[name].getInputs();
                //     float x = position.getX() + inputs.getMovementX() * Const::PLAYER_SPEED * toCompensate;
                //     float y = position.getY() + inputs.getMovementY() * Const::PLAYER_SPEED * toCompensate;
                //
                //     buffer.nextState[name].setPosition(Position(x, y));
                // }

//                std::cout << "COMPENSATED " << name << " - (" << buffer.nextState[name].getPosition().getX() << ", " << buffer.nextState[name].getPosition().getY() << ")" << std::endl;
            }
        }

        // Refreshes buffer
//        buffer.pastStates[buffer.stateTick/TICKRATE.count()] = buffer.currentState;
//        if (buffer.pastStates.size() >= BUFFER_SIZE) {
//            buffer.pastStates.erase(buffer.pastStates.begin());
//        }
        buffer.stateTick = clockState;
        buffer.currentState = buffer.nextState;
        buffer.nextState.clear();
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
            socket.send(packet, sender.value(), player.port);
        }
        return Err::ERR_NONE;
    }

    return Err::ERR_SERVER_SHUTDOWN;
}
