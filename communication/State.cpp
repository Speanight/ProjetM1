#include "State.hpp"

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

    packet << state.getLastInputsId() << state.getPosition() << state.getRadius() << state.getMode() << state.getTimestamp() << size;

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
    int size;
    int timestamp;
    unsigned int lastInputsId;
//    Input input;

    packet >> lastInputsId >> pos >> radius >> mode >> timestamp;

    state.setLastInputsId(lastInputsId);
    state.setPosition(pos);
    state.setRadius(radius);
    state.setMode(mode);
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