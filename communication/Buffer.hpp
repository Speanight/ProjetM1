//
// Created by OMGiT on 06/03/2026.
//

#ifndef PROJETM1_BUFFER_HPP
#define PROJETM1_BUFFER_HPP

#include <map>
#include <string>
#include <unordered_map>
#include "State.hpp"
#include "../Utils.hpp"
#include "Client.hpp"

class Buffer {
private:
    int amtPastStates = Const::BUFFER_SIZE;
    int currentTick = 0;
    std::map<int, std::unordered_map<std::string, State>> pastStates;
    std::unordered_map<std::string, State> currentState;
    std::unordered_map<std::string, State> nextState;

    std::vector<Player> playerList;

public:
    // Constructors
    Buffer(std::vector<Player> playerList = {});

    // Getters / Setters
    int getCurrentTick();
    std::map<int, std::unordered_map<std::string, State>> getPastStates();
    std::unordered_map<std::string, State> getCurrentState();
    std::unordered_map<std::string, State> getNextState();
    void setNextState(std::unordered_map<std::string, State> state, int clockState);

    // Functions
    std::unordered_map<std::string, State> getTState(int t);
    void refreshBuffer(const Player& player, State state, int clockState);
    void addClient(Player p);
    void setPlayerPosition(std::string name, Position position);
};


#endif //PROJETM1_BUFFER_HPP
