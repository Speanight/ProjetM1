#ifndef CONSTS_H
#define CONSTS_H

#include <string>
#include <chrono>

namespace Const {
    static const std::string SERVER_IP = "127.0.0.1";
    static constexpr unsigned short COMM_PORT_SERVER = 35496;
    static const int SERVER_IP_BYTE1 = 127;
    static const int SERVER_IP_BYTE2 = 0;
    static const int SERVER_IP_BYTE3 = 0;
    static const int SERVER_IP_BYTE4 = 1;

    static const int CONSOLE_LINES = 50;

    constexpr auto TICKRATE = std::chrono::milliseconds(1000 / 2); // Amount of ticks in 1s (1s / tickrate)
    static const int BUFFER_SIZE = 5;

    static const int PLAYER_SPEED = 10;
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
    static const int SHUTDOWN = 0;
    static const int POSITION = 1;
}

namespace Inputs {
    // Movement
    static constexpr int MOVEMENT_X = 0;
    static constexpr int MOVEMENT_Y = 1;

    // Actions
    static constexpr int SHOOT = 2;
}

namespace Compensation {
    static constexpr int EXTRAPOLATION = 0;
    static constexpr int INTRAPOLATION = 1;
}


#endif