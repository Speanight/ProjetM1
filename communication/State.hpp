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
    bool mode;

    Input inputs;

public:
    State();
    State(int timestamp, Position position, Input inputs);
    State(int timestamp, Position position, float radius, Input inputs);
    State(int timestamp, Position position, float radius, bool mode, Input inputs);

    // Getters / Setters
    int getTimestamp() const;
    Position getPosition();
    Input getInputs();
    float getRadius();
    bool getMode();

    void setPosition(Position position);
    void setInputs(Input inputs);
    void setRadius(float radius);
    void setMode(bool mode);
};


#endif //PROJETM1_STATE_HPP
