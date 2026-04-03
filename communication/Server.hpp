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
#include "../ui/Console.hpp"

using namespace Const;

class Server : public ServerUI {
private:
    sf::Clock clock;
    sf::UdpSocket socket;
    Buffer buffer;

    std::atomic<bool> running = true;
    std::thread sendThread;
    std::thread receiveThread;
    std::binary_semaphore semaphore;

    std::unordered_map<unsigned short, Player> clients;
    std::unordered_map<std::string, int> pings;

    int maxPlayers = 2;

    bool gameRunning = false;
    bool demoMode = false;
    bool loop = true;
    int mapID=-1;           // -1 stand for not set, mapID going from [0 -> Const::NB_MAP_ID]

public:
    Server(Console &console, sf::Clock& clock);
    ~Server();

    // Getters/Setters
    void setMaxPlayers(int maxPlayers);
    void setDemoMode(bool demoMode);

    // Functions
    int addClient(const std::string& name, unsigned short port, sf::Color color, short weapon);
    void sendLoop();
    void receiveLoop();
    int shutdown();
};


#endif //PROJETM1_SERVER_HPP
