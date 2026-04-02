#include <iostream>
#include <utility>
#include "Buffer.hpp"

// Constructors
Buffer::Buffer(std::vector<Player> playerList) {
    this->playerList = std::move(playerList);
}

// Getters / Setters
int Buffer::getCurrentTick() {
    return this->currentTick;
}

std::unordered_map<std::string, State> Buffer::getCurrentState() {
    return currentState;
}


void Buffer::removeFromPlayerList(const Player& player) {
    playerList.erase(std::remove_if(playerList.begin(), playerList.end(),
                           [&player](const Player& obj) { return obj.getName() == player.getName(); }),
            playerList.end()
    );
}

void Buffer::addToPlayerList(const Player& player) {
    playerList.push_back(player);
}

void Buffer::updateNextPlayerState(const Player& player, State state) {
    nextState[player.getName()] = std::move(state);
}

// Functions
/**
 * @brief pushes down values in the buffer. next states becomes current, current becomes past, and past is updated if needed.
 *
 * @details
 * "Pushes down" every values. The values in next states are completed if some are missing
 * (according to the player list given at init.), then pushed down as current state.
 * The old current state becomes the latest past state (which shrinks down in size if needed),
 * and the current tick is updated according to the given value.
 * [WARNING] - This function uses std::queue, which means if it is used in a thread, a semaphore
 * should be used to give full priority to the function! Not doing so might cause "random" seg.
 * fault whenever the thread gets cut in the middle of its execution.
 *
 * @param clockState - The new timestamp for the current state.
 */
void Buffer::push(int clockState) {
    for (auto & player : playerList) {
        if (auto search = nextState.find(player.getName()); search == nextState.end()) {
            // If a player isn't found in the next "current state"...
            nextState[player.getName()] = currentState[player.getName()]; // Roll backs to previously known pos.
        }
    }

    pastStates.push_front(currentState);

    if (pastStates.size() > amtPastStates) {
        pastStates.pop_back();
    }
    currentTick = clockState;
    currentState = nextState;
    nextState.clear();
    m.unlock();
}

/**
 * @brief Returns the state at moment N-t (with N being present)
 *
 * @details
 * This function returns a map of the States at a moment N-t. If t is greater than 0, it will return
 * the next state. If it's equal to 0, it will return the current state. If it's smaller than
 * 0, if the size of the past states is greater than the given numbers absolute value, it will
 * return the corresponding past state. ie: having a t-value of -1 will return the first past
 * state, the one just before the current state. \n
 * Note that if the given value is bigger than the size of the past states buffer, it will return
 * an empty map ({}).
 *
 * @param t - Moment of state to get. 0 = present, >0 = future, <0 = past.
 * @return map of States, with player names as keys.
 */
std::unordered_map<std::string, State> Buffer::getTState(int t) {
    if (t == 0) {
        return currentState;
    }
    if (t > 0) {
        return nextState;
    }

    for (auto n : pastStates) {
        t++;

        if (t == 0) {
            return n;
        }
    }
    return {}; // Element not found in past states.
}

State Buffer::getStateAtTimestamp(Player player, int timestamp) {
    // Find corresponding state:
    State state;
    bool found = false;

    if (auto search = nextState.find(player.getName()); search != nextState.end()) {
        if (nextState[player.getName()].getTimestamp() <= timestamp) {
            state = nextState[player.getName()];
            found = true;
        }
    }
    if (!found and timestamp >= currentTick) {
        state = currentState[player.getName()];
        found = true;
    }

    if (!found) {
        for (auto& n : pastStates) {
            if (!found and state.getTimestamp() <= timestamp) {
                found = true;
                state = n[player.getName()];
            }
        }
    }

    if (!found) {
        return {};
    }

    // Re-emulate world state until we get to the corresponding timestamp:
    int stateTimestamp = state.getTimestamp();
    int dt;
    Position p;
    p = state.getPosition();

    if (player.getName() == "Client B") {
        std::cout << "";
    }

    for (auto& [inputsTimestamp, Input] : state.getInputs()) {
        dt =  std::min(inputsTimestamp, timestamp) - stateTimestamp;
        p.move(Input.getMovementX(), Input.getMovementY(), dt);

        stateTimestamp += dt;

        if (stateTimestamp == timestamp) {
            state.setPosition(p);
            return state;
        }
    }

    return state;
}

/**
 * @brief Returns the last state of a given player.
 *
 * @details Need a semaphore, as interrupting a "find" mid-execution might cause a SIGSEGV!
 *
 * @param player - Player to look for.
 * @return corresponding state. Empty ({}) if none correspond.
 */
State Buffer::getLastState(const Player& player) {
    if (!nextState.empty()) {
        if (auto search = nextState.find(player.getName()); search != nextState.end()) {
            return search->second;
        }
    }
    if (auto search = currentState.find(player.getName()); search != currentState.end()) {
        return search->second;
    }
    return {};
}

void Buffer::addInputsToLastState(const Player& player, int timestamp, Input inputs) {
    if (auto search = nextState.find(player.getName()); search != nextState.end()) {
        nextState[player.getName()].addInputs(timestamp, inputs);
    }
    else if (auto search = currentState.find(player.getName()); search != currentState.end()) {
        currentState[player.getName()].addInputs(timestamp, inputs);
    }
}

void Buffer::addClient(Player p) {
    playerList.push_back(p);
}