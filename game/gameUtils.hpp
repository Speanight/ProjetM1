//
// Created by OMGiT on 04/03/2026.
//

#ifndef PROJETM1_GAMEUTILS_HPP
#define PROJETM1_GAMEUTILS_HPP

#include <imgui-SFML.h>
#include "Position.hpp"
#include <cmath>

#include <imgui.h>
#include "../communication/Client.hpp"

void drawPlayer(ImDrawList* draw_list, Player player, ImVec2 min, ImVec2 max);
Position resolveCollision(Position player, Position opponent);
Position smoothenDeplacement(Position p, ImVec2 direction, int timestampPos, int timestampNow);

#endif //PROJETM1_GAMEUTILS_HPP
