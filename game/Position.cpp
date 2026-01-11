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

sf::Packet& Position::operator<<(sf::Packet &packet) {
    return packet << this->x << this->y;
}

sf::Packet& Position::operator>>(sf::Packet &packet) {
    return packet >> this->x >> this->y;
}
