#include "Input.hpp"


Input::Input(unsigned int id, float x, float y, float r, bool mode, bool attack, bool onController) {
    this->id = id;
    this->movementX = x;
    this->movementY = y;
    this->rotate = r;
    this->changeWpn = mode;
    this->mode_enable = true;
    this->attack = attack;
    this->onController = onController;
}

// Getters
unsigned int Input::getId() const {
    return id;
}

float Input::getMovementX() const {
    return movementX;
}

float Input::getMovementY() const {
    return movementY;
}

float Input::getRotate() const {
    return rotate;
}

bool Input::getChangeWpn() const {
    return changeWpn;
}

bool Input::getModeEnable() const {
    return mode_enable;
}

bool Input::getAttack() const {
    return attack;
}

bool Input::getAttackEnable() const {
    return attack_enable;
}

bool Input::getOnController() const {
    return onController;
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

void Input::setChangeWpn(bool changeWpn) {
    this->changeWpn = changeWpn;
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

void Input::setOnController(bool onController) {
    this->onController = onController;
}


void Input::handleInput(int inputCode, float value) {
    switch (inputCode) {
        case Inputs::MOVEMENT_UP:
            if (fabs(value) > Const::CONTROLLER_DEADZONE) {
                movementY -= value*100;
            }
            break;
        case Inputs::MOVEMENT_DOWN:
            if (fabs(value) > Const::CONTROLLER_DEADZONE) {
                movementY += value*100;
            }
            break;
        case Inputs::MOVEMENT_LEFT:
            if (fabs(value) > Const::CONTROLLER_DEADZONE) {
                movementX -= value*100;
            }
            break;
        case Inputs::MOVEMENT_RIGHT:
            if (fabs(value) > Const::CONTROLLER_DEADZONE) {
                movementX += value*100;
            }
            break;
        case Inputs::WPN_CCW:       // Handle the weapon rotation
            rotate += value;        // Move less fast than the player
            break;
        case Inputs::WPN_CW:        // Handle the weapon rotation
            rotate -= value;
            break;
        case Inputs::WPN_CHANGE:    // Signal to change the weapon
            changeWpn = value;
            break;
        case Inputs::ATTACK:        // Signal to attack
            attack = value;         // todo : never used ?
            break;
        default:
            break;
    }
}

sf::Packet& operator<<(sf::Packet &packet, Input inputs) {
    return packet << inputs.getId() << inputs.getMovementX() << inputs.getMovementY()
    << inputs.getRotate() << inputs.getChangeWpn() << inputs.getAttack() << inputs.getOnController();
}

sf::Packet& operator>>(sf::Packet &packet, Input& inputs) {
    unsigned int id;
    float x;
    float y;
    float r;
    bool mode;
    bool attack;
    bool onController;
    packet >> id >> x >> y >> r >> mode >> attack >> onController;

    inputs.setId(id);
    inputs.setMovementX(x);
    inputs.setMovementY(y);
    inputs.setRotate(r);
    inputs.setChangeWpn(mode);
    inputs.setAttack(attack);
    inputs.setOnController(onController);

    return packet;
}

bool Input::operator==(const Input& other) const {
    return this->getMovementX() == other.getMovementX() and this->getMovementY() == other.getMovementY()
    and this->getRotate() == other.getRotate() and this->getChangeWpn() == other.getChangeWpn() and
    this->getAttack() == other.getAttack();
}

Input& Input::operator=(const Input& other) {
    if (this != &other) {
        setId(other.getId());
        setMovementX(other.getMovementX());
        setMovementY(other.getMovementY());
        setRotate(other.getRotate());
        setChangeWpn(other.getChangeWpn());
        setAttack(other.getAttack());
        setOnController(other.getOnController());
        setModeEnable(other.getModeEnable());
    }
}


std::ostream &operator<<(std::ostream& os, const Input& inputs) {
    return os << "#" << inputs.getId() << " x=" << inputs.getMovementX() << "; y=" << inputs.getMovementY() << " ATK=" << inputs.getAttack() << " " << inputs.getRotate() << "°";
}