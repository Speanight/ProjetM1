#include <iostream>
#include "gameUtils.hpp"
#include "../communication/Client.hpp"


void drawErrorScreen(ImDrawList* draw_list, Player, ImVec2 min, ImVec2 max) {
    ImVec2 center = {
        (min.x + max.x) * 0.5f,
        (min.y + max.y) * 0.5f
    };

    float width = max.x - min.x;
    float height = max.y - min.y;

    // ERROR TEXT
    const char* error_text = "ERROR ON THE PLAYER !!";

    float font_size = 40.0f;
    ImFont* font = ImGui::GetFont();

    ImVec2 text_size = font->CalcTextSizeA(
        font_size,
        FLT_MAX,
        0.0f,
        error_text
    );

    ImVec2 text_pos = {
        center.x - text_size.x * 0.5f,
        center.y - text_size.y * 1.5f
    };

    draw_list->AddText(
        font,
        font_size,
        text_pos,
        IM_COL32(255, 0, 0, 255),
        error_text
    );

    // BUTTON
    const char* button_text = "> CLICK HERE TO CLOSE <";

    float btn_font_size = 20.0f;

    ImVec2 btn_text_size = font->CalcTextSizeA(
        btn_font_size,
        FLT_MAX,
        0.0f,
        button_text
    );

    ImVec2 btn_size = {
        btn_text_size.x + 20.0f,
        btn_text_size.y + 10.0f
    };

    ImVec2 btn_min = {
        center.x - btn_size.x * 0.5f,
        center.y + 20.0f
    };

    ImVec2 btn_max = {
        btn_min.x + btn_size.x,
        btn_min.y + btn_size.y
    };

    // Button background
    draw_list->AddRectFilled(
        btn_min,
        btn_max,
        IM_COL32(150, 0, 0, 255),
        5.0f
    );

    // Button border
    draw_list->AddRect(
        btn_min,
        btn_max,
        IM_COL32(255, 255, 255, 255),
        5.0f,
        0,
        2.0f
    );

    // Button text
    ImVec2 btn_text_pos = {
        center.x - btn_text_size.x * 0.5f,
        btn_min.y + (btn_size.y - btn_text_size.y) * 0.5f
    };

    draw_list->AddText(
        font,
        btn_font_size,
        btn_text_pos,
        IM_COL32(255, 255, 255, 255),
        button_text
    );

    // INTERRACTION ZONE
    ImGui::SetCursorScreenPos(btn_min);
    if (ImGui::InvisibleButton("error_btn", btn_size)) {
        std::cout<<"CLICK ON THE ERROR BUTTON"<<std::endl;
        // TODO : safelly delete the element on the serv, the clients and stuff while clicking on this button
        std::exit(0);
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

/**
 * Make the angle be in the [0-2pi] interval so he can be easilly calculated
 * @param a angle that need to be normalized
 * @return an angle beetween [0 - 2pi]
 */
float normalize(float a) {
    a = std::fmod(a, 2 * std::numbers::pi);
    if (a < 0) a += 2 * std::numbers::pi;
    return a;
};

short resolveAttacks(State attacker, State opponent) {
    bool blocked = false;

    if (attacker.getWpn().getId() == Weapons::SHIELD) {
        return -1;
    }
    // distance between the players weapon and the surface of the opponent
    ImVec2 dir = {std::cos(attacker.getRadius()), std::sin(attacker.getRadius())};

    switch (attacker.getWpn().getType()) {
        case Weapons::TRIANGLE : {

            ImVec2 attackerPos = {
                attacker.getPosition().getX(),
                attacker.getPosition().getY()
            };

            ImVec2 enemyPos = {
                opponent.getPosition().getX(),
                opponent.getPosition().getY()
            };

            // checking for direction the player is attacking
            ImVec2 toEnemy = {
                enemyPos.x - attackerPos.x,
                enemyPos.y - attackerPos.y
            };

            float distToEnemy = std::sqrt(toEnemy.x*toEnemy.x + toEnemy.y*toEnemy.y);

            if (distToEnemy == 0)
                return -1;

            // normalisation
            toEnemy.x /= distToEnemy;
            toEnemy.y /= distToEnemy;

            // dot product
            float dot = toEnemy.x * dir.x + toEnemy.y * dir.y;

            if (dot < 0.5f)
                return -1; // not hiting in the right direction

            // shield check
            if (opponent.getWpn().getId() == Weapons::SHIELD) {

                // angle de l'attaquant par rapport au centre de l'opposant
                float attackToOpponent = std::atan2(
                    attackerPos.y - enemyPos.y,
                    attackerPos.x - enemyPos.x
                );

                attackToOpponent = normalize(attackToOpponent);

                float opponentAngle = normalize(opponent.getRadius());
                float shieldStart = normalize(opponentAngle - 0.8f);
                float shieldEnd   = normalize(opponentAngle + 0.8f);

                if (shieldStart < shieldEnd)
                    blocked = (attackToOpponent >= shieldStart && attackToOpponent <= shieldEnd);
                else
                    blocked = (attackToOpponent >= shieldStart || attackToOpponent <= shieldEnd);

            }

            // check distance between players
            float attackReach =
                PLAYER_SIZE +
                attacker.getWpn().getHeight() +
                attacker.getWpn().getRange();

            float hitDistance = distToEnemy - PLAYER_SIZE;

            if (hitDistance <= attackReach)
                if(blocked){return 1;}
                else {return 0;}

            return -1;
        }
        case Weapons::RECTANGLE : {
            ImVec2 attackerPos = {
                attacker.getPosition().getX(),
                attacker.getPosition().getY()
            };

            ImVec2 opponentPos = {
                opponent.getPosition().getX(),
                opponent.getPosition().getY()
            };

            // ======== DISTANCE CHECK ========
            float dx = opponentPos.x - attackerPos.x;
            float dy = opponentPos.y - attackerPos.y;
            float distance = std::sqrt(dx*dx + dy*dy);

            float maxReach = PLAYER_SIZE + attacker.getWpn().getHeight();

            if (distance > maxReach + PLAYER_SIZE * 0.8f) {
                return -1;
            }

            // ======== ANGLE CHECK ========
            float angleToOpponent = normalize(std::atan2(dy, dx));

            float start = normalize(attacker.getRadius());
            float end   = normalize(attacker.getRadius() + attacker.getWpn().getRange());
            bool inArc = false;

            if (start < end)
                inArc = (angleToOpponent >= start && angleToOpponent <= end);
            else
                inArc = (angleToOpponent >= start || angleToOpponent <= end);

            if (!inArc) {
                return -1;
            }
            // ======== SHIELD CHECK ========
            if (opponent.getWpn().getId() == Weapons::SHIELD) {
                float opponentRadius = normalize(opponent.getRadius());

                float diff = std::fabs(angleToOpponent - opponentRadius);

                // wrap
                diff = std::fmod(diff, 2 * std::numbers::pi);
                if (diff > std::numbers::pi)
                    diff = 2 * std::numbers::pi - diff;

                // checking for the shield
                if (std::fabs(diff - std::numbers::pi/2) <= 0.8f) {
                    return 1; // bloqued
                }
            }

            return 0; // hit
        }
        default : {
            std::cout<<"weapond not recognized of type ="<<attacker.getWpn().getType()<<std::endl;
        }
    }

    return -1;
}