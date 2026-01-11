#ifndef PROJETM1_SERVER_HPP
#define PROJETM1_SERVER_HPP

#include <SFML/Network.hpp>
#include "../Utils.hpp"
#include <iostream>
#include <thread>

using namespace Const;

class Server {
private:
    sf::UdpSocket socket;
    std::thread thread;

public:
    Server();
    ~Server();

    // Functions
    int listen(sf::IpAddress sender);
};


#endif //PROJETM1_SERVER_HPP
