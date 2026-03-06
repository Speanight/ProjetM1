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

//struct Buffer {
//    int stateTick = 0;
//    std::map<int, std::unordered_map<std::string, State>> pastStates;
//    std::unordered_map<std::string, State> currentState;
//    std::unordered_map<std::string, State> nextState;
//    std::unordered_map<std::string, State> bestGuess;
//};

class Server : public ServerUI {
private:
    sf::Clock clock;
    sf::UdpSocket socket;
    Buffer buffer;

    std::unordered_map<int, bool> compensations;

    std::thread sendThread;
    std::thread receiveThread;
    std::binary_semaphore semaphore;

    std::array<sf::Color, 5> colors;
    std::unordered_map<std::string, Player> clients;

    bool loop = true;
    bool newGame = true;

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
    void refreshBuffer(const std::string& client, State state, int clockState);
};


#endif //PROJETM1_SERVER_HPP
