#include "Position.hpp"

// Constructors
Position::Position() {
    x = 0;
    y = 0;
}


Position::Position(float x, float y) {
    this->x = x;
    this->y = y;
}

// Getters
float Position::getX() const {
    return x;
}

float Position::getY() const {
    return y;
}

// Setters
void Position::setX(float x) {
    this->x = x;
}

void Position::setY(float y) {
    this->y = y;
}

sf::Packet& operator<<(sf::Packet &packet, const Position& position) {
    return packet << position.getX() << position.getY();
}

sf::Packet& operator>>(sf::Packet &packet, Position& position) {
    float x;
    float y;
    packet >> x >> y;

    position.setX(x);
    position.setY(y);

    return packet;
}
