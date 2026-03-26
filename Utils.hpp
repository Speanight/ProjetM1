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
    static const int GRAPH_DISPLAY_VALUES = 2000; // Defines how many values are displayed at the same time in server's console. This means console shows the last DISPLAY_VALUES * DISPLAY_MS milliseconds.

    static const float PLAYER_SPEED = .3f;
    static const float PLAYER_RADIUS_SPEED = .003f;
    static const float PLAYER_SIZE = 20.f;
    static const float CONTROLLER_DEADZONE = .2f;
    static const float WEAPON_GRACE_PERCENT = .4f; // [0-1] - Percent of "grace" for weapon hit detection. Value of .5f means hit will be considered if dist. <= 1.5x weapon size.

    static const float MAP_SIZE_X = 500.f;
    static const float MAP_SIZE_Y = 500.f;

    static const int AMT_PLAYERS_MAX = 4;
}

namespace Err {
    /////////////////////
    //// ERROR CODES ////
    /////////////////////
    static constexpr int ERR_NONE = 0;
    static constexpr int ERR_SERVER_INIT = 1;
    static constexpr int ERR_CLIENT_INIT = 2;
    static constexpr int ERR_SERVER_SHUTDOWN = 3;
    static constexpr int PLAYER_STATUS_UNSYNCED = 4;
    // ...
}

namespace Pkt {
    /////////////////////////
    //// PACKETS HEADERS ////
    /////////////////////////
    // SHUTDOWN
    static const short SHUTDOWN       = 1;    // None                                                                         // indicate to the clients to shut down themselve

    // ACKNOLEDGE
    static const short ACK            = 2;    // Pkt << tick                                                                  // make an acknoledge
    // ACK << READY_R                       // tick                                                                         // acknoledge the server that the client know he can start
    // ACK << DEATH                         // tick                                                                         // acknoledge the server that the client know he is dead
    // ACK << END_R                         // tick                                                                         // acknoledge the server that the client know the round is finished

    // ACK << NEW_PLAYER                    // tick                                                                         // acknoledge the client that the server know he exist and have datas


    /// SERVER PACKAGES (what the server send to the clients)
    static const short READY_R        = 3;    // tick << client.[info client 1] << client.[info client2] << [...]             // put the player in position for the round to start
    static const short START_R        = 4;    // tick << amtPlayer                                                            // send the signal so start the fight to the players
    static const short GLOBAL         = 5;    // tick << amtPlayers << client.name << client.position << [...]                // send the position and the information about the players and it's opponent during the game
    static const short DEATH          = 6;    // tick << killerName                                                           // send the signal to a specific player that the player is dead
    static const short WIN            = 7;
    static const short END_R          = 8;    // tick                                                                         // send the signal that the round is finished

    /// CLIENT PACKAGES (what the clients send to the server)
    static const short NEW_PLAYER     = 9;    // tick << client.name << client.color << client.wpn << client.port             // add the new player datas to the server
    static const short WAIT_OPPONENTS = 10;
    static const short WAIT_START_R   = 11;    // tick << client.port                                                          // acknoledge the server that the client is waiting for the round to start
    static const short INPUTS         = 12;   // tick << inputs << client.port                                                // send the new inputs of the player (the mooves, the radius of the weapon, if he is attacking [...]). also acknoledgge the server that they receive the signal to start the fight
    static const short END_GAME       = 13;   // tick << client.port                                                          // send the signal of the end of the game, asking the server to shutdown

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
    static constexpr short WAITING_FOR_INIT = 0; // Waiting for server to acknowledge client.
    static constexpr short WAITING_FOR_OPPONENTS = 1; // Waiting for opponents info.
    static constexpr short READY_TO_START = 2; // Received all opponents info.
    static constexpr short DONE = 3; // Ready and playing.
    static constexpr short DEAD = 4;
    static constexpr short WAITING_FOR_ROUND_START = 5;
    static constexpr short WIN  = 6;                    // PLayer win                                                   UI = win screen
}

namespace Screens {
    static constexpr short TITLE_SCREEN = 0;
    static constexpr short PLAYER_SELECT = 1;
    static constexpr short GAME = 2;
}

namespace Weapons {
    // Types
    static constexpr short TRIANGLE = 0;
    static constexpr short ARC = 1;

    // Specific weapons
    static constexpr short SHIELD = 0;
}

#endif