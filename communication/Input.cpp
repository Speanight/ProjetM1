#include "Input.hpp"

Input::Input() {
    movementX = 0;
    movementY = 0;
    attack = false;
}

Input::Input(float x, float y, bool attack) {
    this->movementX = x;
    this->movementY = y;
    this->attack = attack;
}

// Getters
float Input::getMovementX() {
    return movementX;
}

float Input::getMovementY() {
    return movementY;
}

bool Input::getAttack() {
    return attack;
}

// Setters
void Input::setMovementX(float x) {
    this->movementX = x;
}

void Input::setMovementY(float y) {
    this->movementY = y;
}

void Input::setAttack(bool attack) {
    this->attack = attack;
}

sf::Packet& operator<<(sf::Packet &packet, Input inputs) {
    return packet << inputs.getMovementX() << inputs.getMovementY() << inputs.getAttack();
}

sf::Packet& operator>>(sf::Packet &packet, Input& inputs) {
    float x;
    float y;
    bool attack;
    packet >> x >> y >> attack;

    inputs.setMovementX(x);
    inputs.setMovementY(y);
    inputs.setAttack(attack);

    return packet;
}