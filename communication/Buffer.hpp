#ifndef PROJETM1_BUFFER_HPP
#define PROJETM1_BUFFER_HPP

#include <map>
#include <string>
#include <unordered_map>
#include "State.hpp"
#include "../Utils.hpp"
#include "../game/Weapon.hpp"
#include <queue>
#include "../game/Player.hpp"

class Buffer {
private:
    int amtPastStates = Const::BUFFER_SIZE;
    int currentTick = 0;
    std::deque<std::unordered_map<std::string,State>> pastStates;
    std::unordered_map<std::string, State> currentState;
    std::unordered_map<std::string, State> nextState;

    std::vector<Player> playerList;

public:
    // Constructors
    Buffer(std::vector<Player> playerList = {});

    // Getters / Setters
    int getCurrentTick();
    std::unordered_map<std::string, State> getCurrentState();

    // Functions
    std::unordered_map<std::string, State> getTState(int t);
    State getLastState(const Player& player);
    void updateNextPlayerState(const Player& player, State state);
    void push(int clockState);
    void addClient(Player p);
    void addInputsToLastState(const Player& player, int timestamp, Input inputs);
    State getStateAtTimestamp(Player player, int timestamp);
};


#endif //PROJETM1_BUFFER_HPP
