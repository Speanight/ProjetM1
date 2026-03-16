#include "State.hpp"

State::State() {
    this->timestamp = 0;
}

// State::State(int timestamp, Position position, float radius, bool mode, Input inputs) {
//     this->timestamp = timestamp;
//     this->position = position;
//     this->radius = radius;
//     this->inputs[timestamp] = inputs;
//     this->mode = mode;
// }
//
// State::State(int timestamp, Position position, float radius, bool mode, bool attack, Input inputs) {
//     this->timestamp = timestamp;
//     this->position = position;
//     this->radius = radius;
//     this->inputs[timestamp] = inputs;
//     this->mode = mode;
//     this->attack = attack;
// }

State::State(int timestamp, Position position, float radius, bool mode, bool attack, int wpn_id, Input inputs) {
    this->timestamp = timestamp;
    this->position = position;
    this->radius = radius;
    this->inputs[timestamp] = inputs;
    this->mode = mode;
    this->attack = attack;
    this->wpn = Weapon(wpn_id);

    this->point = 0;
}

State::State(int timestamp, Position position, float radius, bool mode, bool attack, int wpn_id, int point, Input inputs) {
    this->timestamp = timestamp;
    this->position = position;
    this->radius = radius;
    this->inputs[timestamp] = inputs;
    this->mode = mode;
    this->attack = attack;
    this->wpn = Weapon(wpn_id);
    this->point = point;
}

// GETTERS //
int State::getTimestamp() const {
    return timestamp;
}

Position State::getPosition() {
    return position;
}

float State::getRadius() const {
     return radius;
}

bool State::getMode() const {
    return mode;
}

bool State::getAttack() const {
    return attack;
}

Weapon State::getWpn() {
    return wpn;
}

int State::getPoint() const {
    return point;
}

std::map<int,Input> State::getInputs() {
    return inputs;
}

unsigned int State::getLastInputsId() const {
    return lastInputsId;
}

// SETTERS //

void State::setPosition(Position position) {
    this->position.setX(position.getX());
    this->position.setY(position.getY());
}

void State::setRadius(float radius) {
    this->radius = radius;
}

void State::setMode(bool mode) {
    this->mode = mode;
}

void State::setAttack(bool attack) {
    this->attack = attack;
}

void State::setWpn(int wpn_id) {
    if(this->wpn.getId() == wpn_id) {
        return;
    }
    this->wpn.applyID(wpn_id);
}

void State::setPoint(int point) {
    this->point = point;
}

void State::setTimestamp(int timestamp) {
    this->timestamp = timestamp;
}

void State::setLastInputsId(unsigned int id) {
    this->lastInputsId = id;
}

void State::addInputs(int timestamp, Input inputs) {
    this->inputs[timestamp] = inputs;
    if (inputs.getId() != 0) {
        this->lastInputsId = inputs.getId();
    }
}

Input State::getPercentInput(double percent) {
    int begin = this->inputs.begin()->first;
    int diff = this->inputs.end()->first - begin;
    float lastPercent;
    Input lastIn;

    // TODO: Make use of it later (for compensation purposes) - Need optimization!
//    for (auto & [tps, input] : this->inputs) {
//        if ((float) tps / (float) diff > percent) {
//            return lastIn;
//        }
//        lastIn = input;
//    }

    return lastIn;
}

sf::Packet& operator<<(sf::Packet &packet, State state) {
    std::map<int,Input> inputs = state.getInputs();
    int size = inputs.size();

    packet
    << state.getLastInputsId()
    << state.getPosition()
    << state.getRadius()
    << state.getMode()
    << state.getAttack()
    << state.getWpn().getId()
    << state.getPoint()
    << state.getTimestamp()
    << size;

    // TODO_2: Make use of it later (for compensation purposes) - Need optimization!
//    for (auto & [timestamp, input] : inputs) {
//        packet << timestamp << input;
//    }


    return packet;
}

sf::Packet& operator>>(sf::Packet &packet, State& state) {
    Position pos;
    float radius;
    bool mode;
    bool attack;
    int wpnId;
    int size;
    int point;
    int timestamp;
    unsigned int lastInputsId;
//    Input input;

    packet
    >> lastInputsId
    >> pos
    >> radius
    >> mode
    >> attack
    >> wpnId
    >> point
    >> timestamp;

    state.setLastInputsId(lastInputsId);
    state.setPosition(pos);
    state.setRadius(radius);
    state.setMode(mode);
    state.setAttack(attack);
    state.setWpn(wpnId);
    state.setPoint(point);
    state.setTimestamp(timestamp);

    // Get inputs:
    packet >> size;

//    while (size > 0) {
//        packet >> timestamp >> input;
//        state.addInputs(timestamp, input);
//        size--;
//    }

    return packet;
}