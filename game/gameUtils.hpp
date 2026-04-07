#ifndef PROJETM1_GAMEUTILS_HPP
#define PROJETM1_GAMEUTILS_HPP

#include <imgui-SFML.h>
#include "Position.hpp"
#include <cmath>

#include <imgui.h>
#include "../communication/Client.hpp"

/**
 * Used in the draw selector to keep the datas of each player preset
 */
struct Preset {
    std::string name;
    int color;
    int weapon;
};

void drawErrorScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max);

void drawFightingScreen(ImDrawList* draw_list, const Player player, std::map<std::string, Player> opponents, ImVec2 min, ImVec2 max, int mapID);
void drawPlayer(ImDrawList* draw_list, const Player player, ImVec2 min, ImVec2 max);
void drawWeapon(Player player, ImDrawList* draw_list, ImVec2 pl_position, float scale);

Position resolveCollision(Position player, Position opponent);

float normalize(float a);
short resolveAttacks(State attacker, State opponent);

#endif //PROJETM1_GAMEUTILS_HPP
