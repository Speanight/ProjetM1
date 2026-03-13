#include "Position.hpp"

// Constructors
Position::Position() {
    x = 0;
    y = 0;
}


Position::Position(float x, float y) {
    setX(x);
    setY(y);
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
    if (x < 0) {
        x = 0;
    }
    if (x > Const::MAP_SIZE_X) {
        x = Const::MAP_SIZE_X;
    }
    this->x = x;
}

void Position::setY(float y) {
    if (y < 0) {
        y = 0;
    }
    if (y > Const::MAP_SIZE_Y) {
        y = Const::MAP_SIZE_Y;
    }
    this->y = y;
}

/**
 * @short moves player thanks to inputs and elapsed time.
 *
 * @details
 * Function to centralize the player movement. This uses the input X and input Y and the elapsed
 * time. The inputs should be in the range [-1:1]. The position will then set its X and Y values
 * to the new ones according to the possible deplacement.
 *
 * @param inputX [-1:1] - inputs of the user on the X axis (usually).
 * @param inputY [-1:1] - inputs of the user on the Y axis (usually).
 * @param elapsedTime [in milliseconds] - time passed between last position and the new one. Used to determine distance.
 */
void Position::move(float inputX, float inputY, int elapsedTime) {
    setX(getX() + inputX * Const::PLAYER_SPEED * elapsedTime);
    setY(getY() + inputY * Const::PLAYER_SPEED * elapsedTime);
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

bool Position::operator==(const Position& other) const {
    return this->getX() == other.getX() and this->getY() == other.getY();
}