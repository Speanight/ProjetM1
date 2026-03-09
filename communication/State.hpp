//
// Created by OMGiT on 02/03/2026.
//

#ifndef PROJETM1_STATE_HPP
#define PROJETM1_STATE_HPP

#include "Input.hpp"
#include "../game/Position.hpp"
#include "../Utils.hpp"
#include <unordered_set>
#include <cmath>

class State {
private:
    int timestamp;
    Position position;
    float radius;

    Input inputs;

public:
    State();
    State(int timestamp, Position position, Input inputs);
    State(int timestamp, Position position, float radius, Input inputs);

    // Getters / Setters
    int getTimestamp() const;
    Position getPosition();
    Input getInputs();
    float getRadius();

    void setPosition(Position position);
    void setInputs(Input inputs);
    void setRadius(float radius);
};


#endif //PROJETM1_STATE_HPP
