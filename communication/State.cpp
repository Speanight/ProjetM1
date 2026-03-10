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


void State::setInputs(Input inputs) {
    this->inputs = inputs;
}