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

    float player_radius = Const::PLAYER_RADIUS * scale;

    // Cercle
    draw_list->AddCircleFilled(
        pl_position,
        player_radius,
        IM_COL32(player.color.r, player.color.g, player.color.b, player.color.a)
    );

    // ========= TRIANGLE =========

    float angle = player.radius; // radians

    ImVec2 dir = {
        -sinf(angle),
        -cosf(angle)
    };

    float height = player.wpn.getHeight() * scale;
    float width  = player.wpn.getWidth() * scale;

    float distance = player_radius + 2.f * scale;

    ImVec2 bottom = {
        pl_position.x + dir.x * distance,
        pl_position.y + dir.y * distance
    };

    ImVec2 top = {
        pl_position.x + dir.x * (distance + height),
        pl_position.y + dir.y * (distance + height)
    };

    ImVec2 perp = {
        -dir.y,
         dir.x
    };

    ImVec2 left = {
        bottom.x + perp.x * (width * 0.5f),
        bottom.y + perp.y * (width * 0.5f)
    };

    ImVec2 right = {
        bottom.x - perp.x * (width * 0.5f),
        bottom.y - perp.y * (width * 0.5f)
    };

    draw_list->AddTriangleFilled(
        top,
        left,
        right,
        IM_COL32(player.color.r, player.color.g, player.color.b, player.color.a)
    );
}

Position resolveCollision(Position player, Position opponent) {
    ImVec2 diff = { opponent.getX() - player.getX(),
                    opponent.getY() - player.getY()};

    float distance = sqrtf(pow(diff.x, 2) + pow(diff.y, 2));
    float minDistance = Const::PLAYER_RADIUS + Const::PLAYER_RADIUS;

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

Position smoothenDeplacement(Position p, ImVec2 direction, int timestampPos, int timestampNow) {
    p.setX(p.getX() + direction.x * Const::PLAYER_SPEED * (timestampNow - timestampPos) / 1000);
    p.setY(p.getY() + direction.y * Const::PLAYER_SPEED * (timestampNow - timestampPos) / 1000);

    return p;
}