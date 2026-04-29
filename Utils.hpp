#ifndef CONSTS_H
#define CONSTS_H

#include <string>
#include <chrono>
#include <any>
#include <typeindex>
#include <vector>
#include <SFML/Graphics/Color.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <mutex>
#include <unordered_map>
#include <variant>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <iostream>


extern unsigned short tickrate;
extern unsigned short clientRefreshRate;
extern uint32_t packetID;
extern std::mutex m;
extern std::vector<sf::Texture> GLOBAL_MAP_TEXTURES;

extern std::vector<sf::Texture> textureMaps;

namespace Const {
    static const std::string SERVER_IP = "127.0.0.1";
    static constexpr unsigned short COMM_PORT_SERVER = 35496;
    static const int SERVER_IP_BYTE1 = 127;
    static const int SERVER_IP_BYTE2 = 0;
    static const int SERVER_IP_BYTE3 = 0;
    static const int SERVER_IP_BYTE4 = 1;

    static const int CONSOLE_LINES = 50;

//    constexpr auto TICKRATE = std::chrono::milliseconds(1000 / 10); // Amount of ticks in 1s (1s / tickrate)
    static const int BUFFER_SIZE = 10;
    static const int GRAPH_DISPLAY_MS = 15; // Defines how long (in ms) a "graph line" will be worth for in the server's console.
    static const int GRAPH_DISPLAY_VALUES = 200; // Defines how many values are displayed at the same time in server's console. This means console shows the last DISPLAY_VALUES * DISPLAY_MS milliseconds.

    static const float PLAYER_SPEED = .003f;
    static const float PLAYER_RADIUS_SPEED = .003f;
    static const float PLAYER_SIZE = 20.f;
    static const float CONTROLLER_DEADZONE = .3f;
    static const float WEAPON_GRACE_PERCENT = .8f; // Percent of "grace" for weapon hit detection. Value of .5f means hit will be considered if dist. <= 1.5x weapon size.
    static const int RECONCILIATION_GRACE = 5; // Defines after how many ms. of "desync" the pos. will be adjusted.

    static const float MAP_SIZE_X = 500.f;
    static const float MAP_SIZE_Y = 500.f;
    static const std::vector<std::string> MAP_LINK = {
        "../font/map/dalle1.jpg",
        "../font/map/grass1.jpg",
        "../font/map/grass2.jpg",
    };
    static const int WAIT_RETRY_TIME = 3000;

    static const int AMT_PLAYERS_MAX = 4;

    static const short DISPLAY_REFRESH_RATE = 200;
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
    static const short SHUTDOWN       = 11;    // None                                                                         // indicate to the clients to shut down themselve

    // ACKNOLEDGE
    static const short ACK            = 10;    // Pkt << tick                                                                  // make an acknoledge
    // ACK << READY_R                       // tick                                                                         // acknoledge the server that the client know he can start
    // ACK << DEATH                         // tick                                                                         // acknoledge the server that the client know he is dead
    // ACK << END_R                         // tick                                                                         // acknoledge the server that the client know the round is finished

    // ACK << NEW_PLAYER                    // tick                                                                         // acknoledge the client that the server know he exist and have datas


    /// SERVER PACKAGES (what the server send to the clients)
    static const short READY_R        = 1;    // tick << client.[info client 1] << client.[info client2] << [...]             // put the player in position for the round to start
//    static const short START_R        = 4;    // tick << amtPlayer                                                            // send the signal so start the fight to the players
    static const short GLOBAL         = 2;    // tick << amtPlayers << client.name << client.position << [...]                // send the position and the information about the players and it's opponent during the game
    static const short DEATH          = 3;    // tick << killerName                                                           // send the signal to a specific player that the player is dead
    static const short WIN            = 4;
    static const short END_R          = 5;    // tick                                                                         // send the signal that the round is finished

    /// CLIENT PACKAGES (what the clients send to the server)
    static const short NEW_PLAYER     = 6;    // tick << client.name << client.color << client.wpn << client.port             // add the new player datas to the server
    static const short WAIT_OPPONENTS = 7;
    static const short WAIT_START_R   = 8;    // tick << client.port                                                          // acknoledge the server that the client is waiting for the round to start
    static const short INPUTS         = 9;   // tick << inputs << client.port                                                // send the new inputs of the player (the mooves, the radius of the weapon, if he is attacking [...]). also acknoledgge the server that they receive the signal to start the fight
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

namespace Controller {
    static std::vector<std::unordered_map<int, std::variant<sf::Keyboard::Key, sf::Joystick::Axis, int>>> CONTROLLER_MAP {
        {
            {Inputs::MOVEMENT_DOWN, sf::Joystick::Axis::Y},
            {Inputs::MOVEMENT_RIGHT, sf::Joystick::Axis::X},
            {Inputs::WPN_CCW, 4}, // LB button
            {Inputs::WPN_CW, 5}, // RB button
            {Inputs::WPN_CHANGE, 2}, // X (xbox HyperX)
            {Inputs::ATTACK, sf::Joystick::Axis::R}, // RT button
            {Inputs::WPN_ANGLE_WE, sf::Joystick::Axis::U},
            {Inputs::WPN_ANGLE_NS, sf::Joystick::Axis::V}
        },
        {
            {Inputs::MOVEMENT_DOWN, sf::Joystick::Axis::Y},
            {Inputs::MOVEMENT_RIGHT, sf::Joystick::Axis::X},
            {Inputs::WPN_CCW, 4}, // LB button
            {Inputs::WPN_CW, 5}, // RB button
            {Inputs::WPN_CHANGE, 2}, // X (logitech)
            {Inputs::ATTACK, sf::Joystick::Axis::R}, // RT button
            {Inputs::WPN_ANGLE_WE, sf::Joystick::Axis::U},
            {Inputs::WPN_ANGLE_NS, sf::Joystick::Axis::V}
        }
    };

    static std::vector<std::unordered_map<int, std::variant<sf::Keyboard::Key, sf::Joystick::Axis, int>>> KEYBIND_MAP {
        {
            {Inputs::MOVEMENT_UP, sf::Keyboard::Key::Z},
            {Inputs::MOVEMENT_DOWN, sf::Keyboard::Key::S},
            {Inputs::MOVEMENT_LEFT, sf::Keyboard::Key::Q},
            {Inputs::MOVEMENT_RIGHT, sf::Keyboard::Key::D},
            {Inputs::WPN_CCW, sf::Keyboard::Key::A},
            {Inputs::WPN_CW, sf::Keyboard::Key::E},
            {Inputs::WPN_CHANGE, sf::Keyboard::Key::W},
            {Inputs::ATTACK, sf::Keyboard::Key::C}
        },
        {
            {Inputs::MOVEMENT_UP, sf::Keyboard::Key::Up},
            {Inputs::MOVEMENT_DOWN, sf::Keyboard::Key::Down},
            {Inputs::MOVEMENT_LEFT, sf::Keyboard::Key::Left},
            {Inputs::MOVEMENT_RIGHT, sf::Keyboard::Key::Right},
            {Inputs::WPN_CCW, sf::Keyboard::Key::P},
            {Inputs::WPN_CW, sf::Keyboard::Key::M},
            {Inputs::WPN_CHANGE, sf::Keyboard::Key::L},
            {Inputs::ATTACK, sf::Keyboard::Key::O}
        },
        {
            {Inputs::MOVEMENT_UP, sf::Keyboard::Key::T},
            {Inputs::MOVEMENT_DOWN, sf::Keyboard::Key::G},
            {Inputs::MOVEMENT_LEFT, sf::Keyboard::Key::F},
            {Inputs::MOVEMENT_RIGHT, sf::Keyboard::Key::H},
            {Inputs::WPN_CCW, sf::Keyboard::Key::R},
            {Inputs::WPN_CW, sf::Keyboard::Key::Y},
            {Inputs::WPN_CHANGE, sf::Keyboard::Key::V},
            {Inputs::ATTACK, sf::Keyboard::Key::B}
        },
        {
            {Inputs::MOVEMENT_UP, sf::Keyboard::Key::M},
            {Inputs::MOVEMENT_DOWN, sf::Keyboard::Key::L},
            {Inputs::MOVEMENT_LEFT, sf::Keyboard::Key::K},
            {Inputs::MOVEMENT_RIGHT, sf::Keyboard::Key::J},
            {Inputs::WPN_CCW, sf::Keyboard::Key::H},
            {Inputs::WPN_CW, sf::Keyboard::Key::G},
            {Inputs::WPN_CHANGE, sf::Keyboard::Key::F},
            {Inputs::ATTACK, sf::Keyboard::Key::N}
        }
    };
}

namespace Compensation {
    static constexpr int INTERPOLATION = 0;
    static constexpr int PREDICTION = 1;
    static constexpr int RECONCILIATION = 2;
}

namespace Status {
    static constexpr short WAITING_FOR_INIT = 0;        // Waiting for server to acknowledge client.
    static constexpr short WAITING_FOR_OPPONENTS = 1;   // Waiting for opponents info.
    static constexpr short READY_TO_START = 2;          // Received all opponents info.
    static constexpr short DONE = 3;                    // Ready and playing.
    static constexpr short DEAD = 4;
    static constexpr short WIN  = 5;                    // PLayer win
    static constexpr short END_R = 6;
}

/**
 * different scren that can be showed in general or specifically in the player game
 * [0]      : error screen that is showed when a problem occurs
 * [1-9]    : screens dedicated for the mainWindow
 * [10-19]  : screens dedicated for the game area
 */
namespace Screens {
    static constexpr short ERROR_SCREEN         = 0;

    // General UI to print
    static constexpr short TITLE_SCREEN         = 1;
    static constexpr short GAME_WINDOW          = 2;
    static constexpr short CONFIRM_CLOSE        = 3;

    // Client UI to print
    static constexpr short PLAYER_SELECT        = 10;
    static constexpr short LOADING_SCREEN       = 11;
    static constexpr short GAME                 = 12;
    static constexpr short GAME_WIN             = 13;
    static constexpr short GAME_LOSE            = 14;


}

namespace Weapons {
    // Specific weapons
    static constexpr short SHIELD = 0;

    // Types
    static constexpr short TRIANGLE = 1;
    static constexpr short RECTANGLE = 2;
    static constexpr short CIRCLE = 3;
}

// To use names instead of obscure numbers
namespace Utils {
    static constexpr int RECEIVED = 0;
    static constexpr int SENT = 1;
}

sf::Color convertImUToSfColor(ImU32 im_color);
uint32_t getPacketId();
void loadGlobalMaps();

#endif