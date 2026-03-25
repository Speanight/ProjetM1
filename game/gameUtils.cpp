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

void drawSelectionScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max) {
    // TODO : make it when the player selection menu will be over
}

void drawWaitingScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max) {
    // TODO : maybe not usefull if we choose to draw the player :/
}

void drawFightingScreen(ImDrawList* draw_list, Player player, std::map<std::string, Player> opponents, ImVec2 min, ImVec2 max) {
    // BACKGROUND
    draw_list->AddRectFilled(
        min,
        max,
        IM_COL32(0, 0, 0, 255)
    );

    // TODO : picture / fix the color

    // PLAYERS
    drawPlayer(draw_list,player , min, max);
    for (auto & [name, other] : opponents) {
        drawPlayer(draw_list, other, min, max);
    }
}
void drawPlayer(ImDrawList* draw_list, Player player, ImVec2 min, ImVec2 max) {
    if(player.status == Status::DEAD || player.point == 0) {
        player.color = sf::Color::White;                // TODO : handle the death differently ?
    }
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

void drawEndScreen(ImDrawList* draw_list, Player player, ImVec2 min, ImVec2 max, bool victory) {
    ImVec2 center = {
        (min.x + max.x) * 0.5f,
        (min.y + max.y) * 0.5f
    };

    ImFont* font = ImGui::GetFont();

    float total_height = max.y - min.y;
    float zone_h = total_height / 3.0f;

    float y_title = min.y;
    float y_msg   = min.y + zone_h;
    float y_btn   = min.y + 2.0f * zone_h;

    // SETTING
    const char* title;
    static std::vector<std::string> messages;

    if(victory) {
        title = "VICTOIRE";
        messages = {
            "HONORRRR ! ",
            "La dibidjiii",
            "BOMBE THERMONUCLEAIRE vs BEBE QUI TOUSSE",
            "EPIC WIN",
            "Hehe bah alors, c'est qui le meilleur ?",
            "Wahhh gg bro",
            "TEST 0",
            "TEST 1"
        };
    }
    else {
        title = "DOMMAGE";
        messages = {
            "You can do it !* (*statement of friendship only, you may not be able to do it)",
            "Ah pas loin!",
            "Il a triché t'inquiète",
            "Mais je me fait big gank là c'est quoi ça !",
            "Blammez les JOUEURS pas le jeu...",
            "La prochaine fois c'est la bonne",
            "TEST 0",
            "TEST 1"
        };
    }

    // TITLE
    {
        float title_size = 50.0f;

        ImVec2 title_dim = font->CalcTextSizeA(title_size, FLT_MAX, 0.0f, title);

        ImVec2 title_pos = {
            center.x - title_dim.x * 0.5f,
            y_title + (zone_h - title_dim.y) * 0.5f
        };

        draw_list->AddText(
            font,
            title_size,
            title_pos,
            IM_COL32(player.color.r, player.color.g, player.color.b, 255),
            title
        );
    }

    // PLAYER INFO
    {
        // TODO
    }

    // RANDOM PRAISE
    {
        static int selected = player.port % messages.size();
        std::string msg = messages[selected];

        float msg_size = 18.0f;

        ImVec2 msg_dim = font->CalcTextSizeA(msg_size, FLT_MAX, 0.0f, msg.c_str());

        ImVec2 msg_pos = {
            center.x - msg_dim.x * 0.5f,
            y_msg + (zone_h - msg_dim.y) * 0.5f
        };

        draw_list->AddText(
            font,
            msg_size,
            msg_pos,
            IM_COL32(200, 200, 200, 255),
            msg.c_str()
        );
    }

    // BUTTONS
    {
    float btn_font_size = 22.0f;

    int nb_buttons = 3;
    float gap = 15.0f;

    float total_width = max.x - min.x;

    // largeur boutons
    float usable_width = total_width - (gap * (nb_buttons - 1));
    float btn_width = usable_width / nb_buttons;

    // hauteur basée sur la zone
    float btn_height = 60.0f;

    ImVec2 btn_size = { btn_width, btn_height };

    // centré verticalement dans la zone bouton
    float y = y_btn + (zone_h - btn_height) * 0.5f;

    float start_x = min.x;

    // ===== RETRY =====
    {
        std::string btn_text = "RETRY";

        ImVec2 btn_min = { start_x, y };
        ImVec2 btn_max = { btn_min.x + btn_size.x, btn_min.y + btn_size.y };

        ImGui::SetCursorScreenPos(btn_min);
        if (ImGui::InvisibleButton("btn_retry", btn_size)) {
            player.status = Status::WAITING_FOR_OPPONENTS;
            std::cout<<"CLICK ON "<< btn_text <<std::endl;
        }

        draw_list->AddRectFilled(btn_min, btn_max, IM_COL32(180,180,180,255), 6.0f);
        draw_list->AddRect(btn_min, btn_max, IM_COL32(255,255,255,255), 6.0f, 0, 2.0f);

        ImVec2 text_dim = font->CalcTextSizeA(btn_font_size, FLT_MAX, 0.0f, btn_text.c_str());
        ImVec2 text_pos = {
            btn_min.x + (btn_size.x - text_dim.x) * 0.5f,
            btn_min.y + (btn_size.y - text_dim.y) * 0.5f
        };

        draw_list->AddText(font, btn_font_size, text_pos, IM_COL32(255,255,255,255), btn_text.c_str());
    }

    // ===== CHANGE PLAYER =====
    {
        std::string btn_text = "CHANGE PLAYER";

        ImVec2 btn_min = { start_x + (btn_width + gap), y };
        ImVec2 btn_max = { btn_min.x + btn_size.x, btn_min.y + btn_size.y };

        ImGui::SetCursorScreenPos(btn_min);
        if (ImGui::InvisibleButton("btn_change", btn_size)) {
            player.status = Status::WAITING_FOR_INIT;
            std::cout<<"CLICK ON "<< btn_text <<std::endl;
        }

        draw_list->AddRectFilled(btn_min, btn_max, IM_COL32(180,180,180,255), 6.0f);
        draw_list->AddRect(btn_min, btn_max, IM_COL32(255,255,255,255), 6.0f, 0, 2.0f);

        ImVec2 text_dim = font->CalcTextSizeA(btn_font_size, FLT_MAX, 0.0f, btn_text.c_str());
        ImVec2 text_pos = {
            btn_min.x + (btn_size.x - text_dim.x) * 0.5f,
            btn_min.y + (btn_size.y - text_dim.y) * 0.5f
        };

        draw_list->AddText(font, btn_font_size, text_pos, IM_COL32(255,255,255,255), btn_text.c_str());
    }

    // ===== MENU =====
    {
        std::string btn_text = "MENU";

        ImVec2 btn_min = { start_x + 2 * (btn_width + gap), y };
        ImVec2 btn_max = { btn_min.x + btn_size.x, btn_min.y + btn_size.y };

        ImGui::SetCursorScreenPos(btn_min);
        if (ImGui::InvisibleButton("btn_menu", btn_size)) {
            std::cout<<"CLICK ON "<< btn_text <<std::endl;
        }

        draw_list->AddRectFilled(btn_min, btn_max, IM_COL32(150,0,0,255), 6.0f);
        draw_list->AddRect(btn_min, btn_max, IM_COL32(255,255,255,255), 6.0f, 0, 2.0f);

        ImVec2 text_dim = font->CalcTextSizeA(btn_font_size, FLT_MAX, 0.0f, btn_text.c_str());
        ImVec2 text_pos = {
            btn_min.x + (btn_size.x - text_dim.x) * 0.5f,
            btn_min.y + (btn_size.y - text_dim.y) * 0.5f
        };

        draw_list->AddText(font, btn_font_size, text_pos, IM_COL32(255,255,255,255), btn_text.c_str());
    }
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