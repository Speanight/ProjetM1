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

    bool mode = player.mode;
    float angle = player.radius; // radians
    float distance = player_radius + 2.f * scale;


    // ========= DRAW WEAPON =========

    if (mode) {
        ImVec2 dir = { cosf(angle), sinf(angle) };
        float height = player.wpn.getHeight() * scale;
        float width  = player.wpn.getWidth()  * scale;

        // ======== ATTACK ANIMATION ========
        float offset = 0;
        if(player.timer_atk != -1) {
            if(player.timer_atk <= player.wpn.getAttackSpeed()) {
                offset = player.timer_atk / player.wpn.getAttackSpeed();
            }
            else if(player.timer_atk <= player.wpn.getAttackSpeed() + player.wpn.getReload()) {
                offset = 1 - player.timer_atk / (player.wpn.getAttackSpeed()+ player.wpn.getReload());
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