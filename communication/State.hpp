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
#include <map>

class State {
private:
    int timestamp;
    Position position;
    float radius;
    bool attack;
    Weapon wpn;
    int point;
    unsigned int lastInputsId;

    std::map<int,Input> inputs;

public:
    State();
    State(int timestamp, Position position, Input inputs, float radius = 0, bool attack = false, int wpn_id = 0, int point = 0);

    // Getters / Setters
    int getTimestamp() const;
    Position getPosition() const;
    std::map<int,Input> getInputs() const;
    float getRadius() const;
    unsigned int getLastInputsId() const;
    bool getAttack() const;
    Weapon getWpn() const;
    int getPoint() const;

    void setPosition(Position position);
    void addInputs(int timestamp, Input inputs);
    void setRadius(float radius);
    void setTimestamp(int timestamp);
    void setLastInputsId(unsigned int id);

    Input getPercentInput(double percent);
    void setAttack(bool attack);
    void setWpn(int wpn_id);
    void setPoint(int point);
//    void setTimestamp(int timestamp);

//    void setLastInputsId(unsigned int id);
//    Input getPercentInput(double percent);
};

sf::Packet& operator<<(sf::Packet &packet, State state);
sf::Packet& operator>>(sf::Packet &packet, State& state);

std::ostream &operator<<(std::ostream& os, const State& state);

#endif //PROJETM1_STATE_HPP
