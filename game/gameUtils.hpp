#ifndef PROJETM1_GAMEUTILS_HPP
#define PROJETM1_GAMEUTILS_HPP

#include <imgui-SFML.h>
#include "Position.hpp"
#include <cmath>

#include <imgui.h>
#include "../communication/Client.hpp"

void drawPlayer(ImDrawList* draw_list, Player player, ImVec2 min, ImVec2 max);
void drawWeapon(Player player, ImDrawList* draw_list, ImVec2 pl_position, float scale);
Position resolveCollision(Position player, Position opponent);
short resolveAttacks(State attacker, State opponent);

#endif //PROJETM1_GAMEUTILS_HPP
