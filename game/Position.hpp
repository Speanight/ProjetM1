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

    // Packets
    sf::Packet& operator <<(sf::Packet& packet);
    sf::Packet& operator >>(sf::Packet& packet);
};


#endif //PROJETM1_POSITION_HPP
