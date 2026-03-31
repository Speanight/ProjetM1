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
    std::unordered_map<std::string, int> pings;

    int maxPlayers = 2;     // TODO : make something better to handle how many players we want before starting the game

    bool demo_mode = false;
    bool loop = true;
    bool newGame = false;
    bool newRound = true;   // TODO : put to false at the begining and changing
    bool endGame = false;
    int mapID=-1;           // -1 stand for not set, mapID going from [0 -> Const::NB_MAP_ID]

public:
    Server(sf::Clock clock);
    ~Server();

    // Getters / Setters
    std::unordered_map<std::string, Player> getClients();
    int getMapID();

    // Functions
    int addClient(std::unordered_map<std::string, std::any> infos);
    int sendLoop();
    int receiveLoop();
    int shutdown();
};


#endif //PROJETM1_SERVER_HPP
