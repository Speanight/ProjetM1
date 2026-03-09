#ifndef PROJETM1_CLIENT_H
#define PROJETM1_CLIENT_H

#include "../Utils.hpp"
#include "../game/Position.hpp"
#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/Packet.hpp>
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
#include "../game/Weapon.hpp"
#include "../communication/Buffer.hpp"
#include <cmath>

using namespace Const;

struct QueuedPacket {
    sf::Packet packet;
    sf::Time timestamp;
};

struct NetworkState {
    int ping;
    int packetLoss;

    std::unordered_map<int, bool> compensations;
};


class Client {
private:
    Player player;
    std::deque<QueuedPacket> packets;

    sf::IpAddress server;
    sf::UdpSocket socket;
    std::unordered_map<int,sf::Keyboard::Key> keybinds;

    std::thread sendThread;
    std::thread receiveThread;

    NetworkState network;

    bool loop = true;
    bool newGame = false;


protected:
    sf::Clock clock;
    std::map<std::string, Player> opponents;
    Buffer bufferOnReceipt;

    int lastDisplayedTick;

public:
    Client(sf::Clock clock, std::string name, sf::Color color = sf::Color::Red);
    // Client(sf::Clock clock, std::string name, sf::Color color = sf::Color::Red, float radius = 0);
    ~Client();

    // Copy constructors
    Client(const Client& other);
    Client& operator=(const Client& other);

    // Getters / Setters
    Player getPlayer();
    std::string getName();
    int getPacketLoss() const;
    int getPing() const;
    Input getInputs();
    Position getPosition();
    sf::Color getColor();
    std::unordered_map<int,bool> getCompensations() const;

    void setPacketLoss(int packetLoss);
    void setPing(int ping);
    void setKeybinds(std::unordered_map<int,sf::Keyboard::Key> keybinds);
    void setPosition(Position p);
    void setRadius(float radius);
    void setCompensations(std::unordered_map<int,bool> compensations);

    // Functions
    std::unordered_map<std::string, std::any> init();

    void changeCompensation(int compensation, bool value);

    void move(ImVec2 direction, float deltaTime);

    int sendLoop();
    int receiveLoop();
    void updateLoop();
    std::optional<sf::Packet> getLatestPacket();
};



#endif //PROJETM1_CLIENT_H