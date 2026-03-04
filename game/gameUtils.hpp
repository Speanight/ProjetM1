//
// Created by OMGiT on 04/03/2026.
//

#ifndef PROJETM1_GAMEUTILS_HPP
#define PROJETM1_GAMEUTILS_HPP

#include <imgui-SFML.h>
#include "Position.hpp"
#include <cmath>

#include <imgui.h>

void drawPlayer(ImDrawList* draw_list, Position p, sf::Color c, ImVec2 min, ImVec2 max);
Position resolveCollision(Position player, Position opponent);

#endif //PROJETM1_GAMEUTILS_HPP
