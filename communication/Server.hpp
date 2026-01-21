#ifndef PROJETM1_SERVER_HPP
#define PROJETM1_SERVER_HPP

#include <SFML/Network.hpp>
#include "../Utils.hpp"
#include "../game/Position.hpp"
#include <iostream>
#include <unordered_map>
#include <any>
#include <thread>
#include <chrono>
#include "../ui/ServerUI.hpp"

using namespace Const;


class Server : public ServerUI {
private:
    std::chrono::time_point<std::chrono::steady_clock> clock;
    sf::UdpSocket socket;
    std::thread thread;
    std::array<sf::Color, 5> colors;
    std::unordered_map<std::string, unsigned short> clients;

public:
    Server(std::chrono::time_point<std::chrono::steady_clock> clock);

    ~Server();

    // Getters / Setters
    std::unordered_map<std::string, unsigned short> getClients();

    // Functions
    int addClient(std::unordered_map<std::string, std::any> infos);
    int updateLoop();
    int shutdown();
};


#endif //PROJETM1_SERVER_HPP
