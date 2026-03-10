#include "Input.hpp"

Input::Input() {
    movementX = 0;      // default position = (0.0)
    movementY = 0;
    rotate = 0;         // default rotation = 0
    mode = false;        // default mode SIGNAL = no change
    mode_enable = true; // default mode to access the signal
    attack = false;     // default state = not attacking
}

Input::Input(float x, float y, bool attack) {
    this->movementX = x;
    this->movementY = y;
    this->rotate = 0;
    this->mode = false;
    this->mode_enable = true;
    this->attack = attack;
}

Input::Input(float x, float y, float r, bool attack) {
    this->movementX = x;
    this->movementY = y;
    this->rotate = r;
    this->mode = false;
    this->mode_enable = true;
    this->attack = attack;
}

Input::Input(float x, float y, float r, bool mode, bool attack) {
    this->movementX = x;
    this->movementY = y;
    this->rotate = r;
    this->mode = mode;
    this->mode_enable = true;
    this->attack = attack;
}

// Getters
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

// Setters
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
        case Inputs::WPN_CCW:   // Handle the weapon rotation
            rotate += value;    // Move less fast than the player
            break;
        case Inputs::WPN_CW:    // Handle the weapon rotation
            rotate -= value;
            break;
        case Inputs::WPN_CHANGE:    // Signal to change the weapon
            mode = value;
            break;
        case Inputs::ATTACK:    // Signal to attack
            attack = value;
            break;
        default:
            break;
    }
}

sf::Packet& operator<<(sf::Packet &packet, Input inputs) {
    // return packet << inputs.getMovementX() << inputs.getMovementY() << inputs.getRotate() << inputs.getAttack();
    return packet << inputs.getMovementX() << inputs.getMovementY() << inputs.getRotate() << inputs. getMode() << inputs.getAttack();
}

sf::Packet& operator>>(sf::Packet &packet, Input& inputs) {
    float x;
    float y;
    float r;
    bool mode;
    bool attack;
    packet >> x >> y >> r >> mode >> attack;

    inputs.setMovementX(x);
    inputs.setMovementY(y);
    inputs.setRotate(r);
    inputs.setMode(mode);
    inputs.setAttack(attack);

    return packet;
}