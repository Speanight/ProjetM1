#ifndef PROJETM1_CLIENT_H
#define PROJETM1_CLIENT_H

#include "../game/Position.hpp"
#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/Packet.hpp>
#include "../Utils.hpp"
#include <iostream>

using namespace Const;

class Client {
private:
    sf::UdpSocket socket;
    sf::IpAddress server;

public:
    Client();

    void sendData();
};



#endif //PROJETM1_CLIENT_H