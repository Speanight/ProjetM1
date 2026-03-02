#ifndef PROJETM1_POSITION_HPP
#define PROJETM1_POSITION_HPP

#include <SFML/Network/Packet.hpp>
#include <unordered_map>
#include "../Utils.hpp"

class Position {
private:
    int x;
    int y;

public:
    // Constructors
    Position();
    Position(int x, int y);

    // Getters
    int getX() const;
    int getY() const;

    // Setters
    void setX(int x);
    void setY(int y);
};

sf::Packet& operator<<(sf::Packet& packet, const Position& position);
sf::Packet& operator>>(sf::Packet& packet, Position& position);

#endif //PROJETM1_POSITION_HPP
