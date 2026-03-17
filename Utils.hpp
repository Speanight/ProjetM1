#ifndef CONSTS_H
#define CONSTS_H

#include <string>
#include <chrono>
#include <any>
#include <typeindex>

namespace Const {
    static const std::string SERVER_IP = "127.0.0.1";
    static constexpr unsigned short COMM_PORT_SERVER = 35496;
    static const int SERVER_IP_BYTE1 = 127;
    static const int SERVER_IP_BYTE2 = 0;
    static const int SERVER_IP_BYTE3 = 0;
    static const int SERVER_IP_BYTE4 = 1;

    static const int CONSOLE_LINES = 50;

    constexpr auto TICKRATE = std::chrono::milliseconds(1000 / 10); // Amount of ticks in 1s (1s / tickrate)
    static const int BUFFER_SIZE = 5;
    static const int GRAPH_DISPLAY_MS = 1; // Defines how long (in ms) a "graph line" will be worth for in the server's console.
    static const int GRAPH_DISPLAY_VALUES = 5000; // Defines how many values are displayed at the same time in server's console. This means console shows the last DISPLAY_VALUES * DISPLAY_MS milliseconds.

    static const float PLAYER_SPEED = .3f;
    static const float PLAYER_RADIUS_SPEED = .003f;
    static const float PLAYER_SIZE = 20.f;

    static const float MAP_SIZE_X = 500.f;
    static const float MAP_SIZE_Y = 500.f;
}

namespace Err {
    /////////////////////
    //// ERROR CODES ////
    /////////////////////
    static constexpr int ERR_NONE = 0;
    static constexpr int ERR_SERVER_INIT = 1;
    static constexpr int ERR_CLIENT_INIT = 2;
    static constexpr int ERR_SERVER_SHUTDOWN = 3;
    // ...
}

namespace Pkt {
    /////////////////////
    // PACKETS HEADERS //
    /////////////////////
    static const int SHUTDOWN = 0;      // None
    static const int ROUND_START = 1;   // tick << amtPlayers << client.name << client.position
    static const int GLOBAL = 2;        // tick << amtPlayers << client.name << client.position << [...]
    static const int POSITION = 3;      // tick << client.position
    static const int INPUTS = 4;        // tick << inputs
    static const int ACK = 5;
}

namespace Inputs {
    // Movement
    static constexpr int MOVEMENT_UP = 0;
    static constexpr int MOVEMENT_DOWN = 1;
    static constexpr int MOVEMENT_LEFT = 2;
    static constexpr int MOVEMENT_RIGHT = 3;
    static constexpr int WPN_CCW = 4;
    static constexpr int WPN_CW = 5;
    static constexpr int WPN_ANGLE_WE = 6;
    static constexpr int WPN_ANGLE_NS = 7;
    static constexpr int WPN_CHANGE = 8;
    static constexpr int ATTACK = 9;
}

namespace Compensation {
    static constexpr int INTERPOLATION = 0;
    static constexpr int PREDICTION = 1;
    static constexpr int RECONCILIATION = 2;
}

namespace Status {
    static constexpr short WAITING_FOR_ROUND_START = 0;
    static constexpr short READY_TO_START = 1;
    static constexpr short DONE = 2;
}

#endif