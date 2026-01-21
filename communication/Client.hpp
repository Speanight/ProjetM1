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
#include <SFML/Graphics/Color.hpp>

using namespace Const;

class Client {
private:
    std::chrono::time_point<std::chrono::steady_clock> clock;
    std::string name;
    sf::Color color;
    std::thread thread;
    unsigned short port;
    sf::UdpSocket socket;
    sf::IpAddress server;

    int packetLoss;
    int ping;

public:
    Client(std::chrono::time_point<std::chrono::steady_clock> clock, std::string name, sf::Color color = sf::Color::Red);
    ~Client();

    // Getters / Setters
    std::string getName();
    unsigned short getPort();
    int getPacketLoss() const;
    void setPacketLoss(int packetLoss);
    int getPing() const;
    void setPing(int ping);


    std::unordered_map<std::string, std::any> init();

    void updateLoop();
};



#endif //PROJETM1_CLIENT_H