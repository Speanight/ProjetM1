#include "ClientUI.hpp"

ClientUI::ClientUI(const sf::Clock clock, std::string name, short controller, sf::Color color) : Client(clock, name, controller, color) {}

void ClientUI::drawGame() { // Game space
    const char* title = getName().c_str();

    ImVec2 avail = ImGui::GetContentRegionAvail();

    float size = std::min(avail.x, avail.y);
    size = std::max(size, 400.f); // MINIMUM GAME SIZE


    float offsetX = (avail.x - size) * 0.5f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

    std::string childId = std::string(title) + "##game";
    ImGui::BeginChild(childId.c_str(), ImVec2(size, size), true);

    ImVec2 childMin = ImGui::GetWindowPos();
    ImVec2 childMax = {
        childMin.x + size,
        childMin.y + size
    };
    // ========= DRAW =========
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto player = getPlayer();
    switch (player.getStatus()) {
        case Status::WAITING_FOR_INIT    : {
            std::cout<<"CREATION PLAYER PAGE"<<std::endl;
        }
        case Status::WAITING_FOR_OPPONENTS or Status::READY_TO_START : {
            std::cout<<"LOADING SCREEN / TRAINING ZONE"<<std::endl;
            break;
        }
        case Status::DONE   : {
            // ALIVE SECTION
            drawFightingScreen(draw_list, player, opponents, childMin, childMax);
            break;
        }
        case Status::DEAD : {
            drawEndScreen(draw_list, childMin, childMax, false);
            break;
        }
        case Status::WIN : {
            // TODO [delete me in future merge] - will be triggered with the "WIN" status
            drawEndScreen(draw_list, childMin, childMax, true);
            break;
        }
        default         : {
            drawErrorScreen(draw_list, player, childMin, childMax);
            break;
        }
    }

    ImGui::EndChild();
}

/**
 * Draws the configuration that shows at the top of the player's screen. It is used to
 * change ping and packet loss values, as well as change compensation enabled/disabled.
 */
void ClientUI::drawConfig() {
    const char* title = getName().c_str();

    std::string childId = std::string(title) + "##config";
    ImGui::BeginChild(childId.c_str(), ImVec2(0, 130), true);
    ImGui::Text("%s", title);
    ImGui::Separator();

    int packetLoss[2] = {getReceivingPacketLoss(), getSendingPacketLoss()};
    int ping[2] = {getReceivingPing(), getSendingPing()};
    std::array<bool,3> compensations = getCompensations();

    // Ensure the ping sliders only take available space:
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    ImGui::SliderInt("Packet loss S -> C", &packetLoss[0], 0, 100);
    ImGui::SliderInt("Packet loss C -> S", &packetLoss[1], 0, 100);
    ImGui::PopItemWidth();
//    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
//    ImGui::InputInt("Ping S -> C", &ping[0]);
//    ImGui::SameLine();
    ImGui::InputInt("Ping C -> S", &ping[1]);
//    ImGui::PopItemWidth();

    ImGui::Checkbox("Interpolation", &compensations[Compensation::INTERPOLATION]);
    ImGui::SameLine();
    ImGui::Checkbox("Prediction", &compensations[Compensation::PREDICTION]);
    ImGui::SameLine();
    ImGui::Checkbox("Reconciliation", &compensations[Compensation::RECONCILIATION]);

    setReceivingPacketLoss(packetLoss[0]);
    setSendingPacketLoss(packetLoss[1]);
    setReceivingPing(ping[0]);
    setSendingPing(ping[1]);

    if (!compensations[Compensation::PREDICTION] and compensations[Compensation::RECONCILIATION]) {
        compensations[Compensation::PREDICTION] = true;
    }

    setCompensations(compensations);

    ImGui::EndChild();
}

void ClientUI::addOpponent(const std::string& name, sf::Color color) {
    Player pl;
    pl.setName(name);
    pl.setColor(color);
    opponents.insert(std::make_pair(name, pl));
    this->bufferOnReceipt.addClient(pl);
}

void ClientUI::drawSelectionScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max) {
    // TODO : make it when the player selection menu will be over
}

void ClientUI::drawWaitingScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max) {
    // TODO : maybe not usefull if we choose to draw the player :/
}

void ClientUI::drawFightingScreen(ImDrawList* draw_list, Player player, std::map<std::string, Player> opponents, ImVec2 min, ImVec2 max) {
    // BACKGROUND
    draw_list->AddRectFilled(
        min,
        max,
        IM_COL32(120, 120, 120, 255)
    );

    // TODO : picture / fix the color

    // PLAYERS
    drawPlayer(draw_list,player , min, max);
    for (auto & [name, other] : opponents) {
        drawPlayer(draw_list, other, min, max);
    }
}
void ClientUI::drawPlayer(ImDrawList* draw_list, Player player, ImVec2 min, ImVec2 max) {
    if(player.getStatus() == Status::DEAD || player.getPoint() == 0) {
        player.setColor(sf::Color::White);                // TODO : handle the death differently ?
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
    std::string points = std::to_string(player.getPoint());

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
void ClientUI::drawWeapon(Player player, ImDrawList* draw_list, ImVec2 pl_position, float scale) {
    float player_radius = Const::PLAYER_SIZE * scale;
    float distance = player_radius + 2.f * scale;
    switch (player.getWpn().getType()) {
        case Weapons::TRIANGLE: {
            ImVec2 dir = {cosf(player.getRadius()), sinf(player.getRadius())};
            float height = player.getWpn().getHeight() * scale;
            float width = player.getWpn().getWidth() * scale;

            // ======== ATTACK ANIMATION ========
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

        case Weapons::ARC: {
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

        default:
            std::cout << "Unknown weapon type: " << player.getWpn().getType() << std::endl;
    }
}

void ClientUI::drawEndScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max, bool victory) {
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
            "You can do it !*\n (*statement of friendship only, you may not be able to do it)",
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
            IM_COL32(this->getColor().r, this->getColor().g, this->getColor().b, 255),
            title
        );
    }

    // PLAYER INFO
    {
        // TODO
    }

    // RANDOM PRAISE
    {
        static int selected = this->getPlayer().getPort() % messages.size();
        std::string msg = messages[selected];

        float msg_size = 18.0f;

        float wrap_width = (max.x - min.x) * 0.8f;
        ImVec2 text_dim = font->CalcTextSizeA(
            msg_size,
            wrap_width,
            0.0f,
            msg.c_str()
        );

        ImVec2 center = {
            (min.x + max.x) * 0.5f,
            y_msg + zone_h * 0.5f
        };

        ImVec2 msg_pos = {
            center.x - text_dim.x * 0.5f,
            center.y - text_dim.y * 0.5f
        };

        draw_list->AddText(
            font,
            msg_size,
            msg_pos,
            IM_COL32(200, 200, 200, 255),
            msg.c_str(),
            nullptr,
            wrap_width
        );
    }

    // BUTTONS
    {
        float btn_font_size = 22.0f;
        int nb_buttons = 3;
        float gap = 15.0f;

        // buttons width
        float total_width = max.x - min.x;
        float side_margin = 20.0f;

        float usable_width = total_width - (gap * (nb_buttons - 1)) - 2 * side_margin;
        float btn_width = usable_width / nb_buttons;

        float start_x = min.x + side_margin;

        // btn_height
        float btn_height = 60.0f;
        ImVec2 btn_size = { btn_width, btn_height };

        float y = y_btn + (zone_h - btn_height) * 0.5f;

        // ===== RETRY =====
        {
            std::string btn_text = "RETRY";

            ImVec2 btn_min = { start_x, y };
            ImVec2 btn_max = { btn_min.x + btn_size.x, btn_min.y + btn_size.y };

            ImGui::SetCursorScreenPos(btn_min);
            if (ImGui::InvisibleButton("btn_retry", btn_size)) {
                // player.status = Status::WAITING_FOR_OPPONENTS;
                std::cout<<"CLICK ON "<< btn_text <<std::endl;
            }

            draw_list->AddRectFilled(btn_min, btn_max, IM_COL32(180,180,180,180), 6.0f);                    // content
            draw_list->AddRect(btn_min, btn_max, IM_COL32(255,255,255,255), 6.0f, 0, 2.0f);     // borders

            ImVec2 text_dim = font->CalcTextSizeA(btn_font_size, FLT_MAX, 0.0f, btn_text.c_str());
            ImVec2 text_pos = {
                btn_min.x + (btn_size.x - text_dim.x) * 0.5f,
                btn_min.y + (btn_size.y - text_dim.y) * 0.5f
            };

            draw_list->AddText(font, btn_font_size, text_pos, IM_COL32(255,255,255,255), btn_text.c_str());
        }

        // ===== CHANGE PLAYER =====
        {
            std::string btn_text = "CHANGE\nPLAYER";

            ImVec2 btn_min = { start_x + (btn_width + gap), y };
            ImVec2 btn_max = { btn_min.x + btn_size.x, btn_min.y + btn_size.y };

            ImGui::SetCursorScreenPos(btn_min);
            if (ImGui::InvisibleButton("btn_change", btn_size)) {
                // player.status = Status::WAITING_FOR_INIT;
                std::cout<<"CLICK ON "<< btn_text <<std::endl;
            }

            draw_list->AddRectFilled(btn_min, btn_max, IM_COL32(180,180,180,180), 6.0f);                // content
            draw_list->AddRect(btn_min, btn_max, IM_COL32(255,255,255,255), 6.0f, 0, 2.0f); // border

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
