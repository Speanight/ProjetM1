#ifndef PROJETM1_SERVER_HPP
#define PROJETM1_SERVER_HPP

#include "../Utils.hpp"
#include <iostream>

#include <SFML/Network.hpp>
#include <SFML/System/Clock.hpp>

#include <map>
#include <unordered_map>
#include <any>
#include <thread>
#include <semaphore>

#include "../ui/ServerUI.hpp"
#include "../game/Position.hpp"
#include "../ui/ClientUI.hpp"
#include "State.hpp"
#include "Buffer.hpp"

using namespace Const;

class Server : public ServerUI {
private:
    sf::Clock clock;
    sf::UdpSocket socket;
    Buffer buffer;

    std::thread sendThread;
    std::thread receiveThread;
    std::binary_semaphore semaphore;

    std::unordered_map<std::string, Player> clients;

    bool loop = true;
    bool newGame = false;
    bool newRound = true;   // TODO : put to false at the begining and changing
    bool endGame = false;

public:
    Server(sf::Clock clock);
    ~Server();

    // Getters / Setters
    std::unordered_map<std::string, Player> getClients();

    // Functions
    int addClient(std::unordered_map<std::string, std::any> infos);
    int sendLoop();
    int receiveLoop();
    int shutdown();
};


#endif //PROJETM1_SERVER_HPP
