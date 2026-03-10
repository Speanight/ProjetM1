#include "State.hpp"

#include <utility>

State::State() {
    this->timestamp = 0;
}

State::State(int timestamp, Position position, float radius, bool mode, Input inputs) {
    this->timestamp = timestamp;
    this->position = position;
    this->radius = radius;
    this->inputs[timestamp] = inputs;
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

std::map<int,Input> State::getInputs() {
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

void State::addInputs(int timestamp, Input inputs) {
    this->inputs[timestamp] = inputs;
}

// TODO: operand for packets: [PLACEHOLDERS BELOW]
//sf::Packet& operator<<(sf::Packet &packet, const Position& position) {
//    return packet << position.getX() << position.getY();
//}
//
//sf::Packet& operator>>(sf::Packet &packet, Position& position) {
//    float x;
//    float y;
//    packet >> x >> y;
//
//    position.setX(x);
//    position.setY(y);
//
//    return packet;
//}