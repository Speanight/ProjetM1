#include <iostream>
#include "gameUtils.hpp"
#include "../communication/Client.hpp"


void drawPlayer(ImDrawList* draw_list, Player player, ImVec2 min, ImVec2 max) {
    float window_size = max.x - min.x;

    float scale = window_size / Const::MAP_SIZE_X;

    ImVec2 pl_position = {
        player.position.getX() * scale + min.x,
        player.position.getY() * scale + min.y
    };

    float player_radius = Const::PLAYER_SIZE * scale;

    // ========= PLAYER =========
    draw_list->AddCircleFilled(
        pl_position,
        player_radius,
        IM_COL32(player.color.r, player.color.g, player.color.b, player.color.a)
    );

    drawWeapon(player, draw_list, pl_position, scale);

    // ========= POINT =========
    std::string points = std::to_string(player.point);

    float font_size = 16.0f * scale;

    ImFont* font = ImGui::GetFont();

    ImVec2 text_size = font->CalcTextSizeA(
        font_size,
        FLT_MAX,
        0.0f,
        points.c_str()
    );

    ImVec2 text_pos = {
        pl_position.x - text_size.x * 0.5f,
        pl_position.y - text_size.y * 0.5f
    };

    draw_list->AddText(
        font,
        font_size,
        text_pos,
        IM_COL32(0, 0, 0, 255),
        points.c_str()
    );
}

void drawWeapon(Player player, ImDrawList* draw_list, ImVec2 pl_position, float scale) {
    float player_radius = Const::PLAYER_SIZE * scale;
    float distance = player_radius + 2.f * scale;
    switch (player.wpn.getType()) {
        case Weapons::TRIANGLE: {
            ImVec2 dir = {cosf(player.radius), sinf(player.radius)};
            float height = player.wpn.getHeight() * scale;
            float width = player.wpn.getWidth() * scale;

            // ======== ATTACK ANIMATION ========
            float offset = 0;
            if (player.timer_atk != -1) {
                if (player.timer_atk <= player.wpn.getAttackSpeed()) {
                    offset = player.timer_atk / player.wpn.getAttackSpeed();
                } else if (player.timer_atk <= player.wpn.getAttackSpeed() + player.wpn.getReload()) {
                    offset = 1 - player.timer_atk / (player.wpn.getAttackSpeed() + player.wpn.getReload());
                }
            }

            ImVec2 bottom = {
                    pl_position.x + dir.x * (distance + (offset * player.wpn.getRange()) * scale),
                    pl_position.y + dir.y * (distance + (offset * player.wpn.getRange()) * scale)
            };
            ImVec2 top = {
                    pl_position.x + dir.x * (distance + (offset * player.wpn.getRange()) * scale + height),
                    pl_position.y + dir.y * (distance + (offset * player.wpn.getRange()) * scale + height)
            };

            ImVec2 perp = {-dir.y, dir.x};
            ImVec2 left = {bottom.x + perp.x * (width * 0.5f), bottom.y + perp.y * (width * 0.5f)};
            ImVec2 right = {bottom.x - perp.x * (width * 0.5f), bottom.y - perp.y * (width * 0.5f)};

            draw_list->AddTriangleFilled(
                    top,
                    left,
                    right,
                    IM_COL32(player.color.r, player.color.g, player.color.b, player.color.a)
            );
            break;
        }

        case Weapons::ARC: {
            float arcWidth = 0.8f;
            float a_min = player.radius - arcWidth;
            float a_max = player.radius + arcWidth;

            draw_list->PathArcTo(
                    pl_position,
                    distance + 1.f*scale,
                    a_min,
                    a_max,
                    24
            );

            draw_list->PathStroke(
                    IM_COL32(player.color.r, player.color.g, player.color.b, player.color.a),
                    false,
                    4.f * scale
            );
            break;
        }

        default:
            std::cout << "Unknown weapon type: " << player.wpn.getType() << std::endl;
    }
}

Position resolveCollision(Position player, Position opponent) {
    ImVec2 diff = { opponent.getX() - player.getX(),
                    opponent.getY() - player.getY()};

    float distance = sqrtf(pow(diff.x, 2) + pow(diff.y, 2));
    float minDistance = Const::PLAYER_SIZE + Const::PLAYER_SIZE;

    if (distance < minDistance)
    {
        if (distance == 0.f)
        {
            diff = {1.f, 0.f};
            distance = 1.f;
        }

        ImVec2 normal = { diff.x / distance, diff.y / distance };
        float penetration = minDistance - distance;

        opponent.setX(opponent.getX() + normal.x * penetration);
        opponent.setY(opponent.getY() + normal.y * penetration);
    }

    return opponent;
}

short resolveAttacks(State attacker, State opponent) {
    if (attacker.getWpn().getId() == Weapons::SHIELD) {
        return -1;
    }
    ImVec2 dir = {std::cos(attacker.getRadius()), std::sin(attacker.getRadius())};

    // distance between the players weapon and the surface of the opponent
    float attackReach = PLAYER_SIZE + attacker.getWpn().getHeight() + attacker.getWpn().getRange();

    ImVec2 d2 = {opponent.getPosition().getX() - attacker.getPosition().getX() - dir.x * attackReach,
                 opponent.getPosition().getY() - attacker.getPosition().getY() - dir.y * attackReach};

    float distTop = std::sqrt(pow(d2.x, 2) + pow(d2.y, 2));

    if (distTop <= PLAYER_SIZE * (1+WEAPON_GRACE_PERCENT)) {        // if the weapon can enter the opponent perimeters, then it's a touch
        bool blocked = false;

        if (opponent.getWpn().getId() == Weapons::SHIELD) {
            // looking for the angle between the player and it's opponent
            float angleToAttacker = std::atan2(
                    attacker.getPosition().getY() - opponent.getPosition().getY(),
                    attacker.getPosition().getX() - opponent.getPosition().getX()
            );

            auto normalize = [&](float a) {
                a = std::fmod(a, 2 * std::numbers::pi);
                if (a < 0) a += 2 * std::numbers::pi;
                return a;
            };

            angleToAttacker = normalize(angleToAttacker);
            float opponentRadius = normalize(opponent.getRadius());

            float shieldStart = normalize(opponentRadius - 0.8f);
            float shieldEnd = normalize(opponentRadius + 0.8f);

            if (shieldStart < shieldEnd)
                blocked = (angleToAttacker >= shieldStart && angleToAttacker <= shieldEnd);
            else
                blocked = (angleToAttacker >= shieldStart || angleToAttacker <= shieldEnd);
        }

        return blocked;
    }
    return -1;
}