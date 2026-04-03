#include <iostream>
#include "gameUtils.hpp"
#include "../communication/Client.hpp"

/**
 * Draw an error screen for the player if we don't know wich screen to print
 * @param draw_list : place where the screen will be printed
 * @param min : point on the top right that define the minimum position of the screen
 * @param max : point at the bottom right that define the maximum position of the screen
 */
void drawErrorScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max) {
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

/**
 * Draw the fighting screen with the different player, the background map and the weapon
 * @param draw_list
 * @param player
 * @param opponents
 * @param min
 * @param max
 * @param mapID
 */
void drawFightingScreen(ImDrawList* draw_list, Player player, std::map<std::string, Player> opponents, ImVec2 min, ImVec2 max, int mapID) {
    // BACKGROUND
    int selected = mapID % GLOBAL_MAP_TEXTURES.size();
    sf::Texture& map = GLOBAL_MAP_TEXTURES[selected];
    draw_list->AddImage(
        (ImTextureID)map.getNativeHandle(),
        ImVec2(min.x, min.y),
        ImVec2(max.x, max.y)
    );

    // PLAYERS
    drawPlayer(draw_list,player , min, max);
    for (auto & [name, other] : opponents) {
        drawPlayer(draw_list, other, min, max);
    }
}
void drawPlayer(ImDrawList* draw_list, Player player, ImVec2 min, ImVec2 max) {
    std::string points;
    if(player.getStatus() == Status::DEAD || player.getPoint() == -1) {
        player.setColor(sf::Color::White);                // TODO : handle the death differently ?
        points = "DEAD";
    }
    else {
        points = std::to_string(player.getPoint());
    }
    float window_size = max.x - min.x;

    float scale = window_size / Const::MAP_SIZE_X;

    ImVec2 pl_position = {
        player.getPosition().getX() * scale + min.x,
        player.getPosition().getY() * scale + min.y
    };

    float player_radius = Const::PLAYER_SIZE * scale;

    // ========= PLAYER =========
    draw_list->AddCircleFilled(
        pl_position,
        player_radius,
        IM_COL32(player.getColor().r, player.getColor().g, player.getColor().b, player.getColor().a)
    );

    drawWeapon(player, draw_list, pl_position, scale);

    // ========= POINT =========

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
    switch (player.getWpn().getType()) {
        case Weapons::TRIANGLE  : {
            ImVec2 dir = {cosf(player.getRadius()), sinf(player.getRadius())};
            float height = player.getWpn().getHeight() * scale;
            float width = player.getWpn().getWidth() * scale;

            // ======== ATTACK ANIMATIONS ========
            float offset = 0;
            if (player.getTimer_atk() != -1) {
                if (player.getTimer_atk() <= player.getWpn().getAttackSpeed()) {
                    offset = player.getTimer_atk() / player.getWpn().getAttackSpeed();
                } else if (player.getTimer_atk() <= player.getWpn().getAttackSpeed() + player.getWpn().getReload()) {
                    offset = 1 - player.getTimer_atk() / (player.getWpn().getAttackSpeed() + player.getWpn().getReload());
                }
            }

            ImVec2 bottom = {
                    pl_position.x + dir.x * (distance + (offset * player.getWpn().getRange()) * scale),
                    pl_position.y + dir.y * (distance + (offset * player.getWpn().getRange()) * scale)
            };
            ImVec2 top = {
                    pl_position.x + dir.x * (distance + (offset * player.getWpn().getRange()) * scale + height),
                    pl_position.y + dir.y * (distance + (offset * player.getWpn().getRange()) * scale + height)
            };

            ImVec2 perp = {-dir.y, dir.x};
            ImVec2 left = {bottom.x + perp.x * (width * 0.5f), bottom.y + perp.y * (width * 0.5f)};
            ImVec2 right = {bottom.x - perp.x * (width * 0.5f), bottom.y - perp.y * (width * 0.5f)};

            draw_list->AddTriangleFilled(
                    top,
                    left,
                    right,
                    IM_COL32(player.getColor().r, player.getColor().g, player.getColor().b, player.getColor().a)
            );
            break;
        }
        case Weapons::CIRCLE: {
            ImVec2 dir = { cosf(player.getRadius()), sinf(player.getRadius()) };

            float height = player.getWpn().getHeight() * scale;
            float range  = player.getWpn().getRange() * scale;

            float offset = 0.0f;

            if (player.getTimer_atk() != -1) {

                // ===== ATTACK PHASE =====
                if (player.getTimer_atk() <= player.getWpn().getAttackSpeed()) {
                    offset = player.getTimer_atk() / player.getWpn().getAttackSpeed();
                }
                // ===== RELOAD PHASE =====
                else if (player.getTimer_atk() <= player.getWpn().getAttackSpeed() + player.getWpn().getReload()) {
                    break;
                }
            }

            ImVec2 center = {
                pl_position.x + dir.x * (distance + offset * range),
                pl_position.y + dir.y * (distance + offset * range)
            };

            float radius = height * 0.5f;

            draw_list->AddCircleFilled(
                center,
                radius,
                IM_COL32(player.getColor().r, player.getColor().g, player.getColor().b, player.getColor().a)
            );

            break;
        }
        case Weapons::RECTANGLE : {
            float baseAngle = player.getRadius();

            float height = player.getWpn().getHeight() * scale;
            float width  = player.getWpn().getWidth() * scale;
            float range  = player.getWpn().getRange();

            float offset = 0.f;

            if (player.getTimer_atk() != -1) {
                float atk = player.getWpn().getAttackSpeed();
                float reload = player.getWpn().getReload();

                if (player.getTimer_atk() <= atk) {
                    offset = player.getTimer_atk() / atk;
                } else if (player.getTimer_atk() <= atk + reload) {
                    offset = 1.f - (player.getTimer_atk() - atk) / reload;
                }
            }

            float angle = baseAngle + offset * range;

            ImVec2 dir = { cosf(angle), sinf(angle) };  // rectangle direction
            ImVec2 perp = { -dir.y, dir.x };

            ImVec2 center = {
                    pl_position.x + dir.x * distance,
                    pl_position.y + dir.y * distance
            };

            ImVec2 p1 = { center.x + perp.x * (width * 0.5f), center.y + perp.y * (width * 0.5f) }; // top left
            ImVec2 p2 = { center.x - perp.x * (width * 0.5f), center.y - perp.y * (width * 0.5f) }; // top right

            ImVec2 p3 = { p2.x + dir.x * height, p2.y + dir.y * height };                           // bottom left
            ImVec2 p4 = { p1.x + dir.x * height, p1.y + dir.y * height };                           // bottom right

            draw_list->AddQuadFilled(
                    p1, p2, p3, p4,
                    IM_COL32(
                            player.getColor().r,
                            player.getColor().g,
                            player.getColor().b,
                            player.getColor().a
                    )
            );

            break;
        }
        /*--------------------------------------------*/
        case Weapons::SHIELD    : {
            float arcWidth = 0.8f;
            float a_min = player.getRadius() - arcWidth;
            float a_max = player.getRadius() + arcWidth;

            draw_list->PathArcTo(
                    pl_position,
                    distance + 1.f*scale,
                    a_min,
                    a_max,
                    24
            );

            draw_list->PathStroke(
                    IM_COL32(player.getColor().r, player.getColor().g, player.getColor().b, player.getColor().a),
                    false,
                    4.f * scale
            );
            break;
        }
        /*--------------------------------------------*/
        default:
            std::cout << "Unknown weapon type: " << player.getWpn().getType() << std::endl;
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
            ImVec2 start = {
                attacker.getPosition().getX(),
                attacker.getPosition().getY()
            };

            ImVec2 enemy = {
                opponent.getPosition().getX(),
                opponent.getPosition().getY()
            };

            float attackReach =
                PLAYER_SIZE +
                attacker.getWpn().getHeight() +
                attacker.getWpn().getRange();

            // weapon direction
            ImVec2 attackDir = dir;

            ImVec2 toEnemy = {
                enemy.x - start.x,
                enemy.y - start.y
            };

            float dist = std::sqrt(toEnemy.x*toEnemy.x + toEnemy.y*toEnemy.y);
            if (dist == 0) return -1;

            toEnemy.x /= dist;
            toEnemy.y /= dist;

            // ===== 1. CHECK DIRECTION =====
            float dot = attackDir.x * toEnemy.x + attackDir.y * toEnemy.y;

            if (dot < 0.8f) // touch direction tolerance
                return -1;

            ImVec2 end = {
                start.x + attackDir.x * attackReach,
                start.y + attackDir.y * attackReach
            };

            ImVec2 AB = { end.x - start.x, end.y - start.y };
            ImVec2 AE = { enemy.x - start.x, enemy.y - start.y };

            float ab2 = AB.x*AB.x + AB.y*AB.y;

            float t = (AE.x*AB.x + AE.y*AB.y) / ab2;

            if (t < 0.0f || t > 1.0f)
                return -1;

            ImVec2 closest = {
                start.x + AB.x * t,
                start.y + AB.y * t
            };

            float dx = closest.x - enemy.x;
            float dy = closest.y - enemy.y;

            float radius = PLAYER_SIZE * (1.0f + WEAPON_GRACE_PERCENT);

            // ===== 3. DISTANCE =====
            if (dx*dx + dy*dy > radius*radius)
                return -1;

            // ===== 4. SHIELD =====
            if (opponent.getWpn().getId() == Weapons::SHIELD) {

                float impactAngle = std::atan2(
                    start.y - enemy.y,
                    start.x - enemy.x
                );

                impactAngle = normalize(impactAngle);

                float opponentAngle = normalize(opponent.getRadius());

                float shieldStart = normalize(opponentAngle - 0.8f);
                float shieldEnd   = normalize(opponentAngle + 0.8f);

                bool blocked;
                if (shieldStart < shieldEnd)
                    blocked = (impactAngle >= shieldStart && impactAngle <= shieldEnd);
                else
                    blocked = (impactAngle >= shieldStart || impactAngle <= shieldEnd);

                if (blocked)
                    return 1;
            }

            return 0;
        }
        case Weapons::CIRCLE : {
            ImVec2 start = {
                attacker.getPosition().getX(),
                attacker.getPosition().getY()
            };

            ImVec2 enemy = {
                opponent.getPosition().getX(),
                opponent.getPosition().getY()
            };

            // ===== 1. DIRECTION CHECK =====
            ImVec2 toEnemy = {
                enemy.x - start.x,
                enemy.y - start.y
            };

            float dist = std::sqrt(toEnemy.x*toEnemy.x + toEnemy.y*toEnemy.y);
            if (dist == 0) return -1;

            toEnemy.x /= dist;
            toEnemy.y /= dist;

            float dot = dir.x * toEnemy.x + dir.y * toEnemy.y;

            if (dot < 0.8f) // weapon touch tolerance
                return -1;

            // ===== 2. TRAJECTORY =====
            float attackReach = 10000.0f; // all direction

            ImVec2 end = {
                start.x + dir.x * attackReach,
                start.y + dir.y * attackReach
            };

            ImVec2 AB = { end.x - start.x, end.y - start.y };
            ImVec2 AE = { enemy.x - start.x, enemy.y - start.y };

            float ab2 = AB.x*AB.x + AB.y*AB.y;

            float t = (AE.x*AB.x + AE.y*AB.y) / ab2;

            if (t < 0.0f)
                return -1;

            ImVec2 closest = {
                start.x + AB.x * t,
                start.y + AB.y * t
            };

            float dx = closest.x - enemy.x;
            float dy = closest.y - enemy.y;

            float radius = PLAYER_SIZE * (1.0f + WEAPON_GRACE_PERCENT);

            if (dx*dx + dy*dy > radius*radius)
                return -1;

            // ===== 3. SHIELD =====
            if (opponent.getWpn().getId() == Weapons::SHIELD) {

                float impactAngle = std::atan2(
                    start.y - enemy.y,
                    start.x - enemy.x
                );

                impactAngle = normalize(impactAngle);

                float opponentAngle = normalize(opponent.getRadius());

                float shieldStart = normalize(opponentAngle - 0.8f);
                float shieldEnd   = normalize(opponentAngle + 0.8f);

                bool blocked;
                if (shieldStart < shieldEnd)
                    blocked = (impactAngle >= shieldStart && impactAngle <= shieldEnd);
                else
                    blocked = (impactAngle >= shieldStart || impactAngle <= shieldEnd);

                if (blocked)
                    return 1;
            }

            return 0;
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
