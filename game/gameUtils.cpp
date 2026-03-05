#include "gameUtils.hpp"
#include "../communication/Client.hpp"


void drawPlayer(ImDrawList* draw_list, Player player, ImVec2 min, ImVec2 max) {
    // Cercle
    draw_list->AddCircleFilled(ImVec2(
        player.position.getX() * (max.x - min.x) / Const::MAP_SIZE_X + min.x,
        player.position.getY() * (max.y - min.y) / Const::MAP_SIZE_Y + min.y),
        Const::PLAYER_RADIUS,
        IM_COL32(int{player.color.r}, int{player.color.g}, int{player.color.b}, int{player.color.a})
        );

    // Triangle
    /*
    float triangleHeight = Const::PLAYER_RADIUS * 0.8f;
    float triangleWidth  = Const::PLAYER_RADIUS * 1.2f;

    ImVec2 top   = { p.getX() * (max.x - min.x) / Const::MAP_SIZE_X + min.x,
                     p.getY() * (max.y - min.y) / Const::MAP_SIZE_Y + min.y - Const::PLAYER_RADIUS - triangleHeight };

    ImVec2 left  = { p.getX() * (max.x - min.x) / Const::MAP_SIZE_X + min.x - triangleWidth/2.f,
                     p.getY() * (max.y - min.y) / Const::MAP_SIZE_Y + min.y - Const::PLAYER_RADIUS };

    ImVec2 right = { p.getX() * (max.x - min.x) / Const::MAP_SIZE_X + min.x + triangleWidth/2.f,
                     p.getY() * (max.y - min.y) / Const::MAP_SIZE_Y + min.y - Const::PLAYER_RADIUS };

    draw_list->AddTriangleFilled(top, left, right, IM_COL32(int{c.r}, int{c.g}, int{c.b}, int{c.a}));
    */

    float angle = player.radius; //must be in radiant

    ImVec2 dir = {
        -sinf(angle),
        -cosf(angle)
    };

    // We can use the trigonometry to define the position of the top point of the wpn
    ImVec2 top = {
        player.position.getX() + dir.x * (player.radius + player.wpn.getHeight()+2.f + player.attackOffset.getX()),
        player.position.getY() + dir.y * (player.radius + player.wpn.getHeight()+2.f + player.attackOffset.getY())
    };


    // then we can define the relative position of the left angle and the rigth angle, but first we need to find the center of the bottom of the triangle
    // for that, we can use
    ImVec2 bottom = {
        player.position.getX() + dir.x * (player.radius+2.f + player.attackOffset.getX()),
        player.position.getY() + dir.y * (player.radius+2.f + player.attackOffset.getY())
    };


    ImVec2 perp = {
        -dir.y,
         dir.x
    };

    //And finally, we can calculate the point to the left and the rigth using this formula
    ImVec2 left = {
        bottom.x + perp.x * (player.wpn.getWidth() / 2.f),
        bottom.y + perp.y * (player.wpn.getWidth() / 2.f)
    };

    ImVec2 right = {
        bottom.x - perp.x * (player.wpn.getWidth() / 2.f),
        bottom.y - perp.y * (player.wpn.getWidth() / 2.f)
    };

    draw_list->AddTriangleFilled(
        top,
        left,
        right,
        IM_COL32(int{player.color.r}, int{player.color.g}, int{player.color.b}, int{player.color.a})
            );
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