#ifndef PROJETM1_BUFFER_HPP
#define PROJETM1_BUFFER_HPP

#include <map>
#include <string>
#include <unordered_map>
#include "State.hpp"
#include "../Utils.hpp"
#include "../game/Weapon.hpp"
#include <queue>

struct Player {
    // ====== SERVER ======
    unsigned short port;        // NEVER MOOVE THIS [use to create the client on the server and must be here

    // ====== BASIC ======
    std::string name;
    sf::Color color;
    Position position;

    // ====== WEAPON ======
    float radius;               // must be saved as radiant so degree * ~1.111111 = radiant
    bool is_attacking;          // indicate if the client is attacking or not
    Weapon wpn;
    Position attackOffset;
};

class Buffer {
private:
    int amtPastStates = Const::BUFFER_SIZE;
    int currentTick = 0;
    std::queue<std::unordered_map<std::string, State>> pastStates;
    std::unordered_map<std::string, State> currentState;
    std::unordered_map<std::string, State> nextState;

    std::vector<Player> playerList;

public:
    // Constructors
    Buffer(std::vector<Player> playerList = {});

    // Getters / Setters
    int getCurrentTick();
    std::queue<std::unordered_map<std::string, State>> getPastStates();
    std::unordered_map<std::string, State> getCurrentState();
    std::unordered_map<std::string, State> getNextState();
    void setNextState(std::unordered_map<std::string, State> state, int clockState);

    // Functions
    std::unordered_map<std::string, State> getTState(int t);
    State getLastState(const Player& player);
    void refreshBuffer(const Player& player, State state, int clockState);
    void addClient(Player p);
    void setPlayerPosition(std::string name, Position position);
};


#endif //PROJETM1_BUFFER_HPP
