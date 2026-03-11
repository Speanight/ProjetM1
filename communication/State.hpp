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
#include <map>

class State {
private:
    int timestamp;
    Position position;
    float radius;
    bool mode;
    unsigned int lastInputsId;

    std::map<int,Input> inputs;

public:
    State();
    State(int timestamp, Position position, float radius, bool mode, Input inputs);

    // Getters / Setters
    int getTimestamp() const;
    Position getPosition();
    std::map<int,Input> getInputs();
    float getRadius();
    bool getMode();
    unsigned int getLastInputsId() const;

    void setPosition(Position position);
    void addInputs(int timestamp, Input inputs);
    void setRadius(float radius);
    void setMode(bool mode);
    void setTimestamp(int timestamp);
    void setLastInputsId(unsigned int id);

    Input getPercentInput(double percent);
};

sf::Packet& operator<<(sf::Packet &packet, State state);
sf::Packet& operator>>(sf::Packet &packet, State& state);


#endif //PROJETM1_STATE_HPP
