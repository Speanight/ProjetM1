#ifndef PROJETM1_SERVER_HPP
#define PROJETM1_SERVER_HPP

#include <SFML/Network.hpp>
#include "../Utils.hpp"
#include "../game/Position.hpp"
#include <iostream>
#include <unordered_map>
#include <any>
#include <thread>

using namespace Const;

class Server {
private:
    sf::UdpSocket socket;
    std::thread thread;
    std::unordered_map<std::string, unsigned short> clients;

public:
    Server();
    ~Server();

    // Functions
    int addClient(std::unordered_map<std::string, std::any> infos);
    int listen();
    int shutdown();
};


#endif //PROJETM1_SERVER_HPP
