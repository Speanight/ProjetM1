#ifndef PROJETM1_CLIENT_H
#define PROJETM1_CLIENT_H

#include "../game/Position.hpp"
#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/Packet.hpp>
#include "../Utils.hpp"
#include <iostream>
#include <unordered_map>
#include <any>

using namespace Const;

class Client {
private:
    std::string name;
    unsigned short port;
    sf::UdpSocket socket;
    sf::IpAddress server;

public:
    Client(std::string name);

    std::unordered_map<std::string, std::any> init();

    void sendData();
};



#endif //PROJETM1_CLIENT_H