#ifndef PROJETM1_POSITION_HPP
#define PROJETM1_POSITION_HPP

#include <SFML/Network/Packet.hpp>

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

    int operator<<(int _cpp_par_) const;
};

sf::Packet& operator<<(sf::Packet& packet, const Position& position);
sf::Packet& operator>>(sf::Packet& packet, Position& position);

#endif //PROJETM1_POSITION_HPP
