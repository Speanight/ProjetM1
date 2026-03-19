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

    /// SERVER PACKAGES (what the server send to the clients)
    static const int SHUTDOWN       = 0;    // None                                                                         // indicate to the clients to shut down themselves
    // static const int ACK_P          = 1;    // tick << client.position << client.radius << [...]                            // add the player to the map on its origin position
    static const int READY_R        = 2;    // tick << client.[info client 1] << client.[info client2] << [...]             // put the player in position for the round to start
    static const int START_R        = 3;    // tick << amtPlayer                                                            // send the signal so start the fight to the players
    static const int GLOBAL         = 4;    // tick << amtPlayers << client.name << client.position << [...]                // send the position and the information about the players and it's opponent during the game
    static const int DEATH          = 5;    // tick << killerName                                                           // send the signal to a specific player that the player is dead
    static const int KILL           = 6;    // tick << killedName                                                           // send the signal to the other player that one player is dead (+ who it was)
    static const int END_R          = 7;    // tick                                                                         // send the signal that the round is finished

    /// CLIENT PACKAGES (what the clients send to the server)
    static const int NEW_PLAYER     = 8;    // tick << client.name << client.color << client.wpn << client.port             // add the new player datas to the server
    static const int WAIT_START_R   = 9;    // tick << client.port                                                          // acknoledge the server that the client is waiting for the round to start
    // static const int ACK_R          = 10;   // tick << client.port                                                          // send the message that the player is in the right position and ready to start
    static const int INPUTS         = 11;   // tick << inputs << client.port                                                // send the new inputs of the player (the mooves, the radius of the weapon, if he is attacking [...]). also acknoledgge the server that they receive the signal to start the fight
    // static const int ACK_D          = 12;   // tick << client.port                                                          // acknoledge that the client know he is dead
    // static const int ACK_END_R      = 13;   // tick << client.port                                                          // acknoledge that the client know the round is finished
    static const int END_GAME       = 14;   // tick << client.port                                                          // send the signal of the end of the game, asking the server to shutdown

    // ACKNOLEDGE
    static const int ACK = 1;



    // TODO : CHANGER LES ACKNOLEDGE


    // TODO : delete the old package types
    static const int ROUND_START = 1;   // tick << amtPlayers << client.name << client.position
    static const int POSITION = 3;      // tick << client.position
    static const int NEW_GAME = 6;
}


// Determine in wich case of the game we are, selecting player, starting a new run, playing ...
namespace PartyState {
    ////////////////
    // GAME STATE //
    ////////////////
    static const int GAME_START = 0;    // when the players are not selectioned yet, loops are not running
    static const int GAME_STOP = 1;     // when there is only one player remaining, we cans stop the server

    static const int RUN_START = 2;     // when we acknoledge the players and positionate them on the rigth place
    static const int RUN_IDLE = 3;      // when the game is playing normally

}

namespace Inputs {
    // Movement
    static constexpr int MOVEMENT_UP = 0;
    static constexpr int MOVEMENT_DOWN = 1;
    static constexpr int MOVEMENT_LEFT = 2;
    static constexpr int MOVEMENT_RIGHT = 3;
    static constexpr int WPN_CCW = 4;
    static constexpr int WPN_CW = 5;
    static constexpr int WPN_CHANGE = 6;
    static constexpr int ATTACK = 7;
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
    static constexpr short DEAD = 3;
}

#endif