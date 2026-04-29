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
 * -> draw the background
 * -> draw the opponent
 * -> draw the weapon
 * @param draw_list     : will contain all the element that will be print at the end of the program
 * @param player        : the actual player (placed before)
 * @param opponents     : list of all the opponent witht their position and data
 * @param min           : point at the top left of the screen to define the size
 * @param max           : point at the bottom right of the screen to define the size
 * @param mapID         : id of the map we will use
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
    float window_size = max.x - min.x;
    float scale = window_size / Const::MAP_SIZE_X;
    ImFont* font = ImGui::GetFont();

    ImVec2 pl_position = {
        player.getPosition().getX() * scale + min.x,
        player.getPosition().getY() * scale + min.y
    };

    float player_radius = Const::PLAYER_SIZE * scale;

    // ========= PLAYER =========
    {
        draw_list->AddCircleFilled(
            pl_position,
            player_radius,
            IM_COL32(player.getColor().r, player.getColor().g, player.getColor().b, player.getColor().a)
        );

        drawWeapon(player, draw_list, pl_position, scale);
    }

    // ========= HEALTH BAR =========
    int hp = player.getPoint();
    {
        float barWidth  = player_radius * 1.2f;
        float barHeight = player_radius * 0.5f;

        ImVec2 barPos = {
            pl_position.x - barWidth * 0.5f,
            pl_position.y - barHeight * 0.5f
        };

        if(hp==-1) {
        }
        else {
            hp = hp%101;
            float hpPercent = hp / 100.0f;

            draw_list->AddRectFilled(
                barPos,
                ImVec2(barPos.x + barWidth, barPos.y + barHeight),
                IM_COL32(80, 80, 80, 200)
            );

            draw_list->AddRectFilled(
                barPos,
                ImVec2(barPos.x + barWidth, barPos.y + barHeight),
                IM_COL32(180, 0, 0, 200)
            );

            draw_list->AddRectFilled(
                barPos,
                ImVec2(barPos.x + barWidth * hpPercent, barPos.y + barHeight),
                IM_COL32(0, 200, 0, 220)
            );

            draw_list->AddRect(
                barPos,
                ImVec2(barPos.x + barWidth, barPos.y + barHeight),
                IM_COL32(0, 0, 0, 255)
            );
        }
    }

    // ========= NAME =========
    {
        std::string name;
        sf::Color color;
        if(hp==-1) {
            name = "DEAD";
            color = sf::Color::Red;
        }
        else {
            name = player.getName();
            if(name != "Client A" && name != "Client B") {
                name = name.substr(0, name.size() - 5); // suppress the port (not esthetic)
            }
            color = sf::Color::White;
        }

        float name_font_size = player_radius;

        ImVec2 name_size = font->CalcTextSizeA(
            name_font_size,
            FLT_MAX,
            0.0f,
            name.c_str()
        );

        ImVec2 name_pos = {
            pl_position.x - name_size.x * 0.5f,
            pl_position.y - player_radius - name_size.y - (player_radius * 0.2f)
        };

        draw_list->AddText(
            font,
            name_font_size,
            name_pos,
            IM_COL32(color.r, color.g, color.b, color.a),
            name.c_str()
        );
    }
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

/**
 * Make the player have smooth collision
 * @param player    : actual player (the one who is mooving, and will affect the others)
 * @param opponent  : position of the opponent affected by the player position
 * @return          : position of the opponent after being affected by the moovement
 */
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
 * @param a     : angle that need to be normalized
 * @return      : an angle beetween [0 - 2pi]
 */
float normalize(float a) {
    a = std::fmod(a, 2 * std::numbers::pi);
    if (a < 0) a += 2 * std::numbers::pi;
    return a;
};

/**
 * Handle the attack between the player and the opponent
 * @param attacker  : player who is attacking
 * @param opponent  : opponent who is being attacked
 * @return          : -1 attack miss ; 0 attack worked ; 1 attack blocked by the shield
 */
short resolveAttacks(State attacker, State opponent) {
    bool blocked = false;

    if (attacker.getWpn().getId() == Weapons::SHIELD) {
        return -1;
    }
    // distance between the players weapon and the surface of the opponent
    ImVec2 dir = {std::cos(attacker.getRadius()), std::sin(attacker.getRadius())};

    switch (attacker.getWpn().getType()) {
        case Weapons::TRIANGLE : {
            ImVec2 player = {
                attacker.getPosition().getX(),
                attacker.getPosition().getY()
            };

            ImVec2 enemy = {
                opponent.getPosition().getX(),
                opponent.getPosition().getY()
            };

            // ===== DISTANCE PLAYER -> ENEMY =====
            float a = std::sqrt(
                (enemy.x - player.x)*(enemy.x - player.x) +
                (enemy.y - player.y)*(enemy.y - player.y)
            );

            // ===== ANGLE =====
            ImVec2 toEnemy = {
                enemy.x - player.x,
                enemy.y - player.y
            };

            float dist = std::sqrt(toEnemy.x*toEnemy.x + toEnemy.y*toEnemy.y);
            toEnemy.x /= dist;
            toEnemy.y /= dist;

            float dot = dir.x * toEnemy.x + dir.y * toEnemy.y;

            if (dot > 1.0f) dot = 1.0f;
            if (dot < -1.0f) dot = -1.0f;

            float angle = std::acos(dot);
            // checking if the angle is in the right plage of hit
            if(angle > 0.8 || angle < -0.8) {
                return -1;
            }

            // ===== HAUTEUR =====
            float h = a * std::sin(angle);

            float enemyRadius = PLAYER_SIZE * (1.0f + WEAPON_GRACE_PERCENT);

            // ===== CHECK HIT =====
            if (a <= 2*enemyRadius) {
                // ===== SHIELD =====
                if (opponent.getWpn().getId() == Weapons::SHIELD) {

                    float attackAngle = std::atan2(
                        player.y - enemy.y,
                        player.x - enemy.x
                    );

                    attackAngle = normalize(attackAngle);

                    float opponentAngle = normalize(opponent.getRadius());

                    float shieldStart = normalize(opponentAngle - 0.8f);
                    float shieldEnd   = normalize(opponentAngle + 0.8f);

                    bool blocked;
                    if (shieldStart < shieldEnd)
                        blocked = (attackAngle >= shieldStart && attackAngle <= shieldEnd);
                    else
                        blocked = (attackAngle >= shieldStart || attackAngle <= shieldEnd);

                    if (blocked) {
                        return 1;
                    }
                }

                return 0;
            }

            return -1;
        }
        case Weapons::CIRCLE : {
            ImVec2 player = {
                attacker.getPosition().getX(),
                attacker.getPosition().getY()
            };

            ImVec2 enemy = {
                opponent.getPosition().getX(),
                opponent.getPosition().getY()
            };

            // ===== DISTANCE PLAYER -> ENEMY =====
            float a = std::sqrt(
                (enemy.x - player.x)*(enemy.x - player.x) +
                (enemy.y - player.y)*(enemy.y - player.y)
            );

            // ===== ANGLE =====
            ImVec2 toEnemy = {
                enemy.x - player.x,
                enemy.y - player.y
            };

            float dist = std::sqrt(toEnemy.x*toEnemy.x + toEnemy.y*toEnemy.y);
            toEnemy.x /= dist;
            toEnemy.y /= dist;

            float dot = dir.x * toEnemy.x + dir.y * toEnemy.y;

            if (dot > 1.0f) dot = 1.0f;
            if (dot < -1.0f) dot = -1.0f;

            float angle = std::acos(dot);
            // checking if the angle is in the right plage of hit
            if(angle > 0.7 || angle < -0.7) {
                return -1;
            }

            // ===== HAUTEUR =====
            float h = a * std::sin(angle);

            float enemyRadius = PLAYER_SIZE * (1.0f + WEAPON_GRACE_PERCENT);

            // ===== CHECK HIT =====
            if (a <= 2*enemyRadius) {
                // ===== SHIELD =====
                if (opponent.getWpn().getId() == Weapons::SHIELD) {

                    float attackAngle = std::atan2(
                        player.y - enemy.y,
                        player.x - enemy.x
                    );

                    attackAngle = normalize(attackAngle);

                    float opponentAngle = normalize(opponent.getRadius());

                    float shieldStart = normalize(opponentAngle - 0.8f);
                    float shieldEnd   = normalize(opponentAngle + 0.8f);

                    bool blocked;
                    if (shieldStart < shieldEnd)
                        blocked = (attackAngle >= shieldStart && attackAngle <= shieldEnd);
                    else
                        blocked = (attackAngle >= shieldStart || attackAngle <= shieldEnd);

                    if (blocked) {
                        return 1;
                    }
                }

                return 0;
            }

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