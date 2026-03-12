#include "State.hpp"

#include <utility>

State::State() {
    this->timestamp = 0;
}

State::State(int timestamp, Position position, bool mode, Input inputs) {
    this->timestamp = timestamp;
    setPosition(position);
    setMode(mode);
    this->inputs = inputs;
}

State::State(int timestamp, Position position, float radius, Input inputs) {
    this->timestamp = timestamp;
    setPosition(position);
    setRadius(radius);
    this->inputs = inputs;
}

State::State(int timestamp, Position position, float radius, bool mode, Input inputs) {
    this->timestamp = timestamp;
    this->position = position;
    this->radius = radius;
    this->inputs = inputs;
    this->mode = mode;
}

State::State(int timestamp, Position position, float radius, bool mode, bool attack, Input inputs) {
    this->timestamp = timestamp;
    this->position = position;
    this->radius = radius;
    this->inputs = inputs;
    this->mode = mode;
    this->attack = attack;
}

State::State(int timestamp, Position position, float radius, bool mode, bool attack, int wpn_id, Input inputs) {
    this->timestamp = timestamp;
    this->position = position;
    this->radius = radius;
    this->inputs = inputs;
    this->mode = mode;
    this->attack = attack;
    this->wpn = Weapon(wpn_id);
}

// GETTERS //
int State::getTimestamp() const {
    return timestamp;
}

Position State::getPosition() {
    return position;
}

float State::getRadius() {
     return radius;
}

bool State::getMode() {
    return mode;
}

bool State::getAttack() {
    return attack;
}

int State::getWpnID() {
    return wpn.getId();
}

Input State::getInputs() {
    return inputs;
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

void State::setWpnID(int wpn_id) {
    if(this->getWpnID() == wpn_id) {
        return;
    }
    this->wpn.applyID(wpn_id);
}

void State::setInputs(Input inputs) {
    this->inputs = inputs;
}