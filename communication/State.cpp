#include "State.hpp"

#include <utility>

State::State() {
    this->timestamp = 0;
}

State::State(int timestamp, Position position, const std::unordered_map<int,int>& inputs, bool hasShot) {
    this->timestamp = timestamp;
    this->position = position;
    this->inputs = inputs;
    this->hasShot = hasShot;
}


// GETTERS //
int State::getTimestamp() const {
    return timestamp;
}

Position State::getPosition() {
    return position;
}

std::unordered_map<int,int> State::getInputs() {
    return inputs;
}

bool State::getHasShot() const {
    return hasShot;
}