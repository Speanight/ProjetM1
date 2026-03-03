//
// Created by OMGiT on 02/03/2026.
//

#ifndef PROJETM1_STATE_HPP
#define PROJETM1_STATE_HPP

#include "Input.hpp"
#include "../game/Position.hpp"
#include "../Utils.hpp"
#include <unordered_set>

class State {
private:
    int timestamp;
    Position position;

    Input inputs;

public:
    State();
    State(int timestamp, Position position, Input inputs);

    // Getters / Setters
    int getTimestamp() const;
    Position getPosition();
    Input getInputs();

    void setPosition(Position position);
    void setInputs(Input inputs);
};


#endif //PROJETM1_STATE_HPP
