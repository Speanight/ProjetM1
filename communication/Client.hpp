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
    sf::Time timestamp;
    sf::Packet packet;
};

struct NetworkState {
    int ping[2] = {0,0};        // [0] = reception ping; [1] = sending ping
    int packetLoss[2] = {0,0};  // [0] = reception packet loss; [1] = sending packet loss

    std::array<bool,3> compensations = {false, false, false};
};


class Client {
private:
    Player player;
    std::deque<QueuedPacket> queuedPackets; // [0] = received; [1] = sent

    std::binary_semaphore semaphore;

    sf::IpAddress server;
    sf::UdpSocket socket;
    std::unordered_map<int, std::variant<sf::Keyboard::Key, sf::Joystick::Axis, int>> keybinds; // int = button ID.
    short controllerNumber = -1;

    std::thread sendThread;
    std::thread receiveThread;

    NetworkState network;

    bool newGame = false;
    bool loop = true;
    bool newRound = false;
    bool endGame = false;

protected:
    sf::Clock clock;
    std::map<std::string, Player> opponents;
    Buffer bufferOnReceipt;

    std::map<unsigned int, State> inputsBuffer;

    int lastServerTick;
    unsigned int lastInputId = 0;
    bool created = false;

    int lastUpdate;

public:
    Client(sf::Clock clock, std::string name, short controller = -1, sf::Color color = sf::Color::Red);
    ~Client();

    // Copy constructors
    Client(const Client& other);
    Client& operator=(const Client& other);

    // Getters / Setters
    Player getPlayer();
    std::string getName();
    int getReceivingPacketLoss() const;
    int getSendingPacketLoss() const;
    int getReceivingPing() const;
    int getSendingPing() const;
    sf::Color getColor();
    std::array<bool,3> getCompensations() const;
    bool getCompensationEnabled(int compensation);
    float getRadius() const;
    Position getPosition() const;
    int getStatus() const;
    bool getLoop() const;


    Input getInputs(bool mode_enable=false, bool attack_enable=true);

    void setReceivingPacketLoss(int packetLoss);
    void setSendingPacketLoss(int packetLoss);
    void setReceivingPing(int ping);
    void setSendingPing(int ping);
    void setKeybinds(std::unordered_map<int, std::variant<sf::Keyboard::Key, sf::Joystick::Axis, int>> keybinds);
    void setPosition(Position p);
    void setRadius(float radius);
    void setStatus(int status);
    void setLoop(bool loop);
    void setCompensations(std::array<bool,3> compensations);
    void setController(short controller);
    void setPlayer(Player player);

    // Functions
    std::unordered_map<std::string, std::any> init();

    [[noreturn]] void update();
    int sendPacket(Input inputs);

    [[noreturn]] void receiveLoop();
    std::optional<sf::Packet> getLatestQueuedPacket();

    void applyState(std::string name, State state);

    // Compensations
    void compensationInterpolation();
    void compensationPrediction(Input inputs);
    void compensationReconciliation();
};

#endif //PROJETM1_CLIENT_H