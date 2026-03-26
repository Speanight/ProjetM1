#ifndef PROJETM1_POSITION_HPP
#define PROJETM1_POSITION_HPP

#include <SFML/Network/Packet.hpp>
#include <unordered_map>
#include "../Utils.hpp"
#include <iostream>

class Position {
private:
    float x;
    float y;

public:
    // Constructors
    Position();
    Position(float x, float y);

    // Getters
    float getX() const;
    float getY() const;

    // Setters
    void setX(float x);
    void setY(float y);

    // Operators
    bool operator==(const Position& other) const;

    void move(float inputX, float inputY, int elapsedTime);
};

sf::Packet& operator<<(sf::Packet& packet, const Position& position);
sf::Packet& operator>>(sf::Packet& packet, Position& position);

std::ostream& operator<<(std::ostream& os, const Position& position);

#endif //PROJETM1_POSITION_HPP
