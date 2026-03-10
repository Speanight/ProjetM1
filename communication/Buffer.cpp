#include <iostream>
#include "Buffer.hpp"

// Constructors
Buffer::Buffer(std::vector<Player> playerList) {
    this->playerList = playerList;
}

// Getters / Setters
int Buffer::getCurrentTick() {
    return this->currentTick;
}

std::queue<std::unordered_map<std::string, State>> Buffer::getPastStates() {
    return pastStates;
}

std::unordered_map<std::string, State> Buffer::getCurrentState() {
    return currentState;
}

std::unordered_map<std::string, State> Buffer::getNextState() {
    return nextState;
}

void Buffer::setNextState(std::unordered_map<std::string, State> state, int clockState) {
    this->currentTick = clockState;
    this->currentState = nextState;
    this->nextState.clear();
    this->nextState = state;
}

void Buffer::updateNextPlayerState(const Player& player, State state) {
    nextState[player.name] = state;
}

void Buffer::push(int clockState) {
    for (auto & player : playerList) {
        if (auto search = nextState.find(player.name); search == nextState.end()) {
            // If a player isn't find in the next "current state"...
            nextState[player.name] = currentState[player.name]; // Roll backs to previously known pos.
        }
    }

    pastStates.push(currentState);

    if (pastStates.size() > amtPastStates) {
        pastStates.pop();
    }
    currentTick = clockState;
    currentState = nextState;
    nextState.clear();
}

bool Buffer::refreshBuffer(const Player& player, State state, int clockState) {
    nextState[player.name] = state;

    if (currentTick / Const::TICKRATE.count() < clockState / Const::TICKRATE.count()) {
        for (auto & player : playerList) {
            if (auto search = nextState.find(player.name); search == nextState.end()) {
                // If a player isn't find in the next "current state"...
                nextState[player.name] = currentState[player.name]; // Roll backs to previously known pos.
            }
        }

        pastStates.push(currentState);

        if (pastStates.size() > amtPastStates) {
            pastStates.pop();
        }

        currentTick = clockState;
        currentState = nextState;
        nextState.clear();

        return true;
    }

    return false;
}

// Functions
std::unordered_map<std::string, State> Buffer::getTState(int t) {
    if (t == 0) {
        return currentState;
    }
    if (t > 0) {
        return nextState;
    }
    std::queue<std::unordered_map<std::string, State>> copyPastStates = pastStates;

    while (!copyPastStates.empty()) {
        t++;
        std::unordered_map<std::string, State> state = copyPastStates.back();
        copyPastStates.pop();

        if (t == 0) {
            return state;
        }
    }
    return {}; // Element not found in past states.
}

State Buffer::getLastState(const Player& player) {
    if (auto search = nextState.find(player.name); search != nextState.end()) {
        return search->second;
    }
    if (auto search = currentState.find(player.name); search != currentState.end()) {
        return search->second;
    }
    return {};
}

std::unordered_map<std::string,State> Buffer::getStateOfTick(int tick) {
    if (currentTick - tick > 0) {
        if (currentTick - tick < Const::TICKRATE.count()) {
            return currentState;
        }
        return nextState;
    }
    return getTState(-(currentTick - tick)/Const::TICKRATE.count());
}

void Buffer::addClient(Player p) {
    playerList.push_back(p);
}

void Buffer::setPlayerPosition(std::string name, Position position) {
    this->currentState[name].setPosition(position);
}