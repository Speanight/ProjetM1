#ifndef PROJETM1_SERVER_HPP
#define PROJETM1_SERVER_HPP

#include <SFML/Network.hpp>
#include "../Utils.hpp"
#include "../game/Position.hpp"
#include <iostream>
#include <unordered_map>
#include <any>
#include <thread>
#include <SFML/System/Clock.hpp>
#include "../ui/ServerUI.hpp"

using namespace Const;


class Server : public ServerUI {
private:
    sf::Clock clock;
    sf::UdpSocket socket;
    std::thread sendThread;
    std::thread receiveThread;
    std::array<sf::Color, 5> colors;
    std::unordered_map<std::string, unsigned short> clients;
    bool loop = true;

public:
    Server(sf::Clock clock);

    ~Server();

    // Getters / Setters
    std::unordered_map<std::string, unsigned short> getClients();

    // Functions
    int addClient(std::unordered_map<std::string, std::any> infos);
    int sendLoop();
    int receiveLoop();
    int shutdown();
};


#endif //PROJETM1_SERVER_HPP
