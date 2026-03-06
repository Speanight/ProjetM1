//
// Created by OMGiT on 06/03/2026.
//

#include "Buffer.hpp"

// Constructors
Buffer::Buffer(std::vector<Player> playerList) {
    this->playerList = playerList;
}

// Getters / Setters
int Buffer::getCurrentTick() {
    return this->currentTick;
}

std::map<int, std::unordered_map<std::string, State>> Buffer::getPastStates() {
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

void Buffer::refreshBuffer(const Player& player, State state, int clockState) {
    nextState[player.name] = state;

    if (currentTick / Const::TICKRATE.count() < clockState / Const::TICKRATE.count()) {
        for (auto & player : playerList) {
            if (auto search = nextState.find(player.name); search == nextState.end()) {
                // If a player isn't find in the next "current state"...
                nextState[player.name] = currentState[player.name]; // Roll backs to previously known pos.
            }
        }

        currentTick = clockState;
        currentState = nextState;
        nextState.clear();
    }
}

// Functions
std::unordered_map<std::string, State> Buffer::getTState(int t) {
    if (t == 0) {
        return currentState;
    }
    else if (t > 0) {
        return nextState;
    }
    else if (-t <= pastStates.size()) {
        return pastStates[-t];
    }
    else {
        return {};
    }
}

void Buffer::addClient(Player p) {
    playerList.push_back(p);
}

void Buffer::setPlayerPosition(std::string name, Position position) {
    this->currentState[name].setPosition(position);
}