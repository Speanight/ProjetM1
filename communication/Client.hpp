#ifndef PROJETM1_CLIENT_H
#define PROJETM1_CLIENT_H

#include "../game/Position.hpp"
#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/Packet.hpp>
#include "../Utils.hpp"
#include <iostream>
#include <unordered_map>
#include <any>
#include <chrono>
#include <thread>
#include <experimental/random>

using namespace Const;

class Client {
private:
    std::chrono::time_point<std::chrono::steady_clock> clock;
    std::string name;
    std::thread thread;
    unsigned short port;
    sf::UdpSocket socket;
    sf::IpAddress server;

    int packetLoss;

public:
    Client(std::chrono::time_point<std::chrono::steady_clock> clock, std::string name);
    ~Client();

    // Getters / Setters
    std::string getName();
    unsigned short getPort();
    int getPacketLoss() const;
    void setPacketLoss(int packetLoss);


    std::unordered_map<std::string, std::any> init();

    void updateLoop();
};



#endif //PROJETM1_CLIENT_H