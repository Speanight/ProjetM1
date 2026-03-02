//
// Created by OMGiT on 02/03/2026.
//

#ifndef PROJETM1_STATE_HPP
#define PROJETM1_STATE_HPP

#include "../game/Position.hpp"
#include "../Utils.hpp"
#include <unordered_set>

class State {
private:
    int timestamp;
    Position position;

    std::unordered_map<int,int> inputs;
    bool hasShot = false;

public:
    State();
    State(int timestamp, Position position, const std::unordered_map<int,int>& inputs = {}, bool hasShot = false);

    // Getters / Setters
    int getTimestamp() const;
    Position getPosition();
    std::unordered_map<int,int> getInputs();
    bool getHasShot() const;
};


#endif //PROJETM1_STATE_HPP
