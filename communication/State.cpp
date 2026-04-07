#include "State.hpp"

State::State() {
    this->timestamp = 0;
}

State::State(int timestamp, Position position, Input inputs, float radius, bool attack, int wpn_id, int point) {
    this->timestamp = timestamp;
    this->position = position;
    this->radius = radius;
    this->inputs[timestamp] = inputs;
    this->attack = attack;
    this->attackTimestamp = -1;
    this->wpn = Weapon(wpn_id);
    this->point = point;
}

// GETTERS //
int State::getTimestamp() const {
    return timestamp;
}

Position State::getPosition() const {
    return position;
}

float State::getRadius() const {
     return radius;
}

bool State::getAttack() const {
    return attack;
}

int State::getAttackTimestamp() const {
    return attackTimestamp;
}

Weapon State::getWpn() const {
    return wpn;
}

int State::getPoint() const {
    return point;
}


std::map<int,Input> State::getInputs() const {
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

void State::setAttack(bool attack) {
    this->attack = attack;
}

void State::setWpn(int wpn_id) {
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

void State::setAttackTimestamp(int timestamp) {
    this->attackTimestamp = timestamp;
}

void State::addInputs(int timestamp, Input inputs) {
    if (!this->inputs.empty()) {
        if (this->inputs.rend()->second != inputs) {
            this->inputs.insert(this->inputs.end(), {timestamp, inputs});
        }
    }
    else {
            this->inputs.insert(this->inputs.end(), {timestamp, inputs});
    }
    if (inputs.getId() != 0) {
        this->lastInputsId = inputs.getId();
    }

    if (timestamp > this->timestamp) {
        this->timestamp = timestamp;
    }
}

Input State::getPercentInput(double percent) {
    float begin = this->inputs.begin()->first;
    float diff = this->timestamp - begin;
    Input lastIn;

    for (auto & [tps, input] : this->inputs) {
        if ((float(tps)-begin)/diff > percent) {
            return input;
        }
        lastIn = input;
    }

    return lastIn;
}

void State::flushInputs() {
    this->inputs.clear();
}

sf::Packet& operator<<(sf::Packet &packet, State state) {
    std::map<int,Input> inputs = state.getInputs();
    int size = inputs.size();

    packet
    << state.getLastInputsId()
    << state.getPosition()
    << state.getRadius()
    << state.getAttack()
    << state.getWpn().getId()
    << state.getPoint()
    << state.getTimestamp()
    << size;

    // TODO_2: Make use of it later (for compensation purposes) - Need optimization!
    for (auto & [timestamp, input] : inputs) {
        packet << int(timestamp) << input;
    }


    return packet;
}

sf::Packet& operator>>(sf::Packet &packet, State& state) {
    Position pos;
    float radius;
    bool attack;
    short wpnId;
    int size;
    int point;
    int timestamp;
    unsigned int lastInputsId;
    Input input;

    packet
    >> lastInputsId
    >> pos
    >> radius
    >> attack
    >> wpnId
    >> point
    >> timestamp;

    state.setLastInputsId(lastInputsId);
    state.setPosition(pos);
    state.setRadius(radius);
    state.setAttack(attack);
    state.setWpn(wpnId);
    state.setPoint(point);
    state.setTimestamp(timestamp);

    // Get inputs:
    packet >> size;

    while (size > 0) {
        packet >> timestamp >> input;
        state.addInputs(timestamp, input);
        size--;
    }

    return packet;
}

std::ostream &operator<<(std::ostream& os, const State& state) {
    return os << "@" << state.getTimestamp() << " " << state.getPosition() << " ATK? = " << state.getAttack() << " " << state.getRadius() << "°";
}