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

Input::Input(float x, float y, float r, float l, bool attack) {
    this->movementX = x;
    this->movementY = y;
    this->rotateRigth = r;
    this->rotateLeft = l;
    this->attack = attack;
}


// Getters
float Input::getMovementX() {
    return movementX;
}

float Input::getMovementY() {
    return movementY;
}

float Input::getRotateRigth() {
    return rotateRigth;
}

float Input::getRotateLeft() {
    return rotateLeft;
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

void Input::setRotateRigth(float r) {
    this->rotateRigth = r;
}

void Input::setRotateLeft(float r) {
    this->rotateLeft = r;
}

void Input::setAttack(bool attack) {
    this->attack = attack;
}

void Input::handleInput(int inputCode, float value) {
    switch (inputCode) {
        case Inputs::MOVEMENT_UP:
            movementY -= value;
            break;
        case Inputs::MOVEMENT_DOWN:
            movementY += value;
            break;
        case Inputs::MOVEMENT_LEFT:
            movementX -= value;
            break;
        case Inputs::MOVEMENT_RIGHT:
            movementX += value;
            break;
        case Inputs::WPN_RIGHT:
            rotate += value * 0.5f; // moove less fast than the player
            break;
        case Inputs::WPN_LEFT:
            rotate -= value * 0.5f;
            break;
        case Inputs::ATTACK:
            attack = true;
            break;
        default:
            break;
    }
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