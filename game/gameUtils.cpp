#include "gameUtils.hpp"

void drawPlayer(ImDrawList* draw_list, Position p, sf::Color c, ImVec2 min, ImVec2 max) {
    // Cercle
    draw_list->AddCircleFilled(ImVec2(p.getX() * (max.x - min.x) / Const::MAP_SIZE_X + min.x, p.getY() * (max.y - min.y) / Const::MAP_SIZE_Y + min.y), Const::PLAYER_RADIUS, IM_COL32(int{c.r}, int{c.g}, int{c.b}, int{c.a}));

    // Triangle
    float triangleHeight = Const::PLAYER_RADIUS * 0.8f;
    float triangleWidth  = Const::PLAYER_RADIUS * 1.2f;

    ImVec2 top   = { p.getX() * (max.x - min.x) / Const::MAP_SIZE_X + min.x,
                     p.getY() * (max.y - min.y) / Const::MAP_SIZE_Y + min.y - Const::PLAYER_RADIUS - triangleHeight };

    ImVec2 left  = { p.getX() * (max.x - min.x) / Const::MAP_SIZE_X + min.x - triangleWidth/2.f,
                     p.getY() * (max.y - min.y) / Const::MAP_SIZE_Y + min.y - Const::PLAYER_RADIUS };

    ImVec2 right = { p.getX() * (max.x - min.x) / Const::MAP_SIZE_X + min.x + triangleWidth/2.f,
                     p.getY() * (max.y - min.y) / Const::MAP_SIZE_Y + min.y - Const::PLAYER_RADIUS };

    draw_list->AddTriangleFilled(top, left, right, IM_COL32(int{c.r}, int{c.g}, int{c.b}, int{c.a}));
}

Position resolveCollision(Position player, Position opponent) {
    ImVec2 diff = { opponent.getX() - player.getX(),
                    opponent.getY() - player.getY()};

    float distance = sqrtf(diff.x*diff.x + diff.y*diff.y);
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

        opponent.setX(player.getX() + normal.x * penetration);
        opponent.setY(player.getY() + normal.y * penetration);
    }

    return opponent;
}

Position checkPositionRealistic(Position posFrom, Position posTo, float timestampFrom, float timestampTo) {
    float distance = sqrt(pow(posTo.getX() - posFrom.getX(), 2) + pow(posTo.getY() - posFrom.getY(), 2));

    // We give a 5% eventual wiggle-room.
    if (distance < timestampTo + Const::TICKRATE.count() *1.05 - timestampFrom) {
        return posTo;
    }
    else {
        float angle = 0;
        return Position();
    }
}