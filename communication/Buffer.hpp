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
    unsigned short status = Status::WAITING_FOR_ROUND_START;

    // ====== BASIC ======
    std::string name;
    sf::Color color;
    Position position = Position();

    // ====== WEAPON ======
    float radius;               // must be saved as radiant so degree * ~1.111111 = radiant
    bool mode;                  // indicate if the weapon is in attack or defense mode (# true = atk, false = defense
    Weapon wpn;

    // ====== ATTACK ======
    bool isAttacking;           // indicate if the player is attacking or not
    int timer_atk;              // timer that indicate where we are in the animation, -1 stand for no animation
    int point;
};

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
    void updateNextPlayerState(const Player& player, State state, bool oldMode = false);
    void push(int clockState);
    void addClient(Player p);
    void addInputsToLastState(const Player& player, int timestamp, Input inputs);
};


#endif //PROJETM1_BUFFER_HPP
