#include "Position.hpp"

// Constructors
Position::Position() {
    x = 0;
    y = 0;
}


Position::Position(int x, int y) {
    this->x = x;
    this->y = y;
}

// Getters
int Position::getX() const {
    return x;
}

int Position::getY() const {
    return y;
}

// Setters
void Position::setX(int x) {
    this->x = x;
}

void Position::setY(int y) {
    this->y = y;
}

sf::Packet& operator<<(sf::Packet &packet, const Position& position) {
    return packet << position.getX() << position.getY();
}

sf::Packet& operator>>(sf::Packet &packet, Position& position) {
    int x;
    int y;
    packet >> x >> y;

    position.setX(x);
    position.setY(y);

    return packet;
}
