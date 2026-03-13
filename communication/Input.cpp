#include "Input.hpp"


Input::Input(unsigned int id, float x, float y, float r, bool mode, bool attack, int wpn_id) {
    this->id = id;
    this->movementX = x;
    this->movementY = y;
    this->rotate = r;
    this->mode = mode;
    this->mode_enable = true;
    this->attack = attack;
    this->wpn_id = wpn_id;
}


// Getters
unsigned int Input::getId() {
    return id;
}

float Input::getMovementX() {
    return movementX;
}

float Input::getMovementY() {
    return movementY;
}

float Input::getRotate() {
    return rotate;
}

bool Input::getMode() {
    return mode;
}

bool Input::getModeEnable() {
    return mode_enable;
}

bool Input::getAttack() {
    return attack;
}

bool Input::getAttackEnable() {
    return attack_enable;
}

int Input::getWpnID() {
    return wpn_id;
}


// Setters
void Input::setId(unsigned int id) {
    this->id = id;
}

void Input::setMovementX(float x) {
    this->movementX = x;
}

void Input::setMovementY(float y) {
    this->movementY = y;
}

void Input::setRotate(float r) {
    this->rotate= r;
}

void Input::setMode(bool mode) {
    this->mode = mode;
}

void Input::setModeEnable(bool mode_enable) {
    this->mode_enable = mode_enable;
}

void Input::setAttack(bool attack) {
    this->attack = attack;
}

void Input::setAttackEnable(bool attack_enable) {
    this->attack_enable = attack_enable;
}

void Input::setWpnID(int wpn_id) {
    this->wpn_id = wpn_id;
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
        case Inputs::WPN_CCW:       // Handle the weapon rotation
            rotate += value;        // Move less fast than the player
            break;
        case Inputs::WPN_CW:        // Handle the weapon rotation
            rotate -= value;
            break;
        case Inputs::WPN_CHANGE:    // Signal to change the weapon
            mode = value;
            break;
        case Inputs::ATTACK:        // Signal to attack
            attack = value;
            break;
        default:
            break;
    }
}

sf::Packet& operator<<(sf::Packet &packet, Input inputs) {
    return packet << inputs.getId() << inputs.getMovementX() << inputs.getMovementY() << inputs.getRotate() << inputs.getMode() << inputs.getAttack() << inputs.getWpnID();
}

sf::Packet& operator>>(sf::Packet &packet, Input& inputs) {
    unsigned int id;
    float x;
    float y;
    float r;
    bool mode;
    bool attack;
    int wpn_id;
    packet >> id >> x >> y >> r >> mode >> attack >> wpn_id;

    inputs.setId(id);
    inputs.setMovementX(x);
    inputs.setMovementY(y);
    inputs.setRotate(r);
    inputs.setMode(mode);
    inputs.setAttack(attack);
    inputs.setWpnID(wpn_id);

    return packet;
}