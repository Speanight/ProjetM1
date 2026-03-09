#include "State.hpp"

#include <utility>

State::State() {
    this->timestamp = 0;
}

State::State(int timestamp, Position position, Input inputs) {
    this->timestamp = timestamp;
    setPosition(position);
    this->inputs = inputs;
}

State::State(int timestamp, Position position, float radius, Input inputs) {
    this->timestamp = timestamp;
    setPosition(position);
    setRadius(radius);
    this->inputs = inputs;
}

// GETTERS //
int State::getTimestamp() const {
    return timestamp;
}

Position State::getPosition() {
    return position;
}

Input State::getInputs() {
    return inputs;
}

float State::getRadius() {
     return radius;
}


void State::setPosition(Position position) {
    this->position.setX(position.getX());
    this->position.setY(position.getY());
}

void State::setInputs(Input inputs) {
    this->inputs = inputs;
}

void State::setRadius(float radius) {
    this->radius = std::fmod(radius, 360);
    while (this->radius < 0) {
        this->radius += 360;
    }
}