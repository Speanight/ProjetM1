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
#include "../game/Weapon.hpp"

class State {
private:
    int timestamp;
    Position position;
    float radius;
    bool mode;
    bool attack;
    Weapon wpn;

    Input inputs;

public:
    State();
    State(int timestamp, Position position, bool mode, Input inputs);
    State(int timestamp, Position position, float radius, Input inputs);
    State(int timestamp, Position position, float radius, bool mode, Input inputs);
    State(int timestamp, Position position, float radius, bool mode, bool attack, Input inputs);
    State(int timestamp, Position position, float radius, bool mode, bool attack, int wpn_id, Input inputs);

    // Getters / Setters
    int getTimestamp() const;
    Position getPosition();
    Input getInputs();
    float getRadius();
    bool getMode();
    bool getAttack();
    int getWpnID();

    void setPosition(Position position);
    void setInputs(Input inputs);
    void setRadius(float radius);
    void setMode(bool mode);
    void setAttack(bool attack);
    void setWpnID(int wpn_id);
};


#endif //PROJETM1_STATE_HPP
