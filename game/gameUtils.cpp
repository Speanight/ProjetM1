#include <iostream>
#include "gameUtils.hpp"
#include "../communication/Client.hpp"


void drawPlayer(ImDrawList* draw_list, Player& player, ImVec2 min, ImVec2 max, const sf::Clock& clk) {
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

    bool mode = player.mode;
    float angle = player.radius; // radians
    float distance = player_radius + 2.f * scale;

    // ========= ATTACK =========
    bool atk = player.isAttacking;

    float t = clk.getElapsedTime().asSeconds();

    if (atk) {
        if(player.end_atk_phase == 0 && player.end_rld_phase == 0) {
            player.end_atk_phase = t + player.wpn.getAttackSpeed();
            player.end_rld_phase = t + player.wpn.getAttackSpeed() + player.wpn.getReload();
        }
    }

    // Calcul de l'offset pour l'animation
    float offset = 0.f;
    if (player.end_atk_phase > t) {
        float attackDuration = player.wpn.getAttackSpeed();
        float elapsed = attackDuration - (player.end_atk_phase - t); // temps écoulé dans l'attaque
        offset = (elapsed / attackDuration) * player.wpn.getRange(); // 0 -> maxRange
    }
    else {
        player.end_atk_phase = 0;
        if (player.end_rld_phase > t) {
            float reloadDuration = player.wpn.getReload();
            float elapsed = reloadDuration - (player.end_rld_phase - t); // temps écoulé dans le reload
            offset = ((reloadDuration - elapsed) / reloadDuration) * player.wpn.getRange(); // maxRange -> 0
        }
        else {
            player.end_rld_phase = 0;
        }
    }

    // ========= DRAW WEAPON =========
    if (mode) {
        ImVec2 dir = { cosf(angle), sinf(angle) };
        float height = player.wpn.getHeight() * scale;
        float width  = player.wpn.getWidth()  * scale;

        // On applique l'offset pour l'animation
        ImVec2 bottom = {
            pl_position.x + dir.x * (distance + offset * scale),
            pl_position.y + dir.y * (distance + offset * scale)
        };
        ImVec2 top = {
            pl_position.x + dir.x * (distance + offset * scale + height),
            pl_position.y + dir.y * (distance + offset * scale + height)
        };

        ImVec2 perp = { -dir.y, dir.x };
        ImVec2 left  = { bottom.x + perp.x * (width*0.5f), bottom.y + perp.y * (width*0.5f) };
        ImVec2 right = { bottom.x - perp.x * (width*0.5f), bottom.y - perp.y * (width*0.5f) };

        draw_list->AddTriangleFilled(
            top,
            left,
            right,
            IM_COL32(player.color.r, player.color.g, player.color.b, player.color.a)
        );
    }
    else {
        // ========= DEFENSE MODE =========
        float arcWidth = 0.8f;
        float a_min = angle - arcWidth;
        float a_max = angle + arcWidth;

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

Position smoothenDeplacement(State p, State s, int clockSync) {
    Position pos;
    pos.setX(p.getPosition().getX() + (s.getPosition().getX() - p.getPosition().getX()) * clockSync / (s.getTimestamp() - p.getTimestamp()));
    pos.setY(p.getPosition().getY() + (s.getPosition().getY() - p.getPosition().getY()) * clockSync / (s.getTimestamp() - p.getTimestamp()));
    return pos;
}