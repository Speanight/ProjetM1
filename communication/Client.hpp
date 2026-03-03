#ifndef PROJETM1_CLIENT_H
#define PROJETM1_CLIENT_H

#include "../game/Position.hpp"
#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/Packet.hpp>
#include "../Utils.hpp"
#include "Input.hpp"
#include <iostream>
#include <unordered_map>
#include <any>
#include <chrono>
#include <thread>
#include <experimental/random>
#include <SFML/Graphics/Color.hpp>
#include <deque>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <imgui-SFML.h>
#include <SFML/Window/Keyboard.hpp>
#include <imgui.h>
#include <map>
#include "../communication/Input.hpp"

using namespace Const;

struct QueuedPacket {
    sf::Packet packet;
    sf::Time timestamp;
};

class Client {
private:
    sf::Clock clock;
    std::string name;
    sf::Color color;
    Position position;
    std::deque<QueuedPacket> packets;

    sf::IpAddress server;
    unsigned short port;
    sf::UdpSocket socket;
    std::unordered_map<int,sf::Keyboard::Key> keybinds;

    std::thread sendThread;
    std::thread receiveThread;

    int packetLoss;
    int ping;
    bool loop = true;

protected:
    float m_radius = 20.f;

public:
    Client(sf::Clock clock, std::string name, sf::Color color = sf::Color::Red);
    ~Client();

    // Copy constructors
    Client(const Client& other);
    Client& operator=(const Client& other);

    // Getters / Setters
    std::string getName();
    int getPacketLoss() const;
    int getPing() const;
    Input getInputs();
    Position getPosition();

    sf::Color getColor();

    void setPacketLoss(int packetLoss);
    void setPing(int ping);
    void setKeybinds(std::unordered_map<int,sf::Keyboard::Key> keybinds);


    // Functions
    std::unordered_map<std::string, std::any> init();

    void move(ImVec2 direction, float deltaTime);
    void clampToChild(ImVec2 childMin, ImVec2 childMax);
    void resolveCollision(Client& other);

    int sendLoop();
    int receiveLoop();
    void updateLoop();
    std::optional<sf::Packet> getLatestPacket();
};



#endif //PROJETM1_CLIENT_H