#include "ClientUI.hpp"

#include <algorithm>

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
    // DRAW
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // TODO : => mooving this switch case in an other loop ? <=
    if(getLoop() == false) {
        screenToShow = Screens::PLAYER_SELECT;
    }
    else {
        auto player = getPlayer();
        // Switching the screen to show depending on the player status
        switch (player.status) {
            case Status::WAITING_FOR_OPPONENTS or Status::READY_TO_START or Status::WAITING_FOR_INIT: {
                screenToShow = Screens::LOADING_SCREEN;
                break;
            }
            case Status::DONE   : {
                screenToShow = Screens::GAME;
                break;
            }
            case Status::DEAD : {
                screenToShow = Screens::GAME_LOSE;
                break;
            }
            case Status::WIN : {
                screenToShow = Screens::GAME_WIN;
                break;
            }
            case Status::END_R : {
                // screenToShow remain the same;
                break;
            }
            default         : {
                std::cout<<"Unknown player status to show"<<std::endl;
                drawErrorScreen(draw_list, player, childMin, childMax);
                break;
            }
        }
    }

    auto player = getPlayer();
    switch (screenToShow) {
        case Screens::PLAYER_SELECT : {
            drawSelectionScreen(draw_list, childMin, childMax);
            break;
        }
        case Screens::LOADING_SCREEN : {
            drawLoadingScreen(draw_list, childMin, childMax);
            break;
        }
        case Screens::GAME : {
            drawFightingScreen(draw_list, player, opponents, childMin, childMax);
            break;
        }
        case Screens::GAME_LOSE : {
            drawEndScreen(draw_list, childMin, childMax, false);
            break;
        }
        case Screens::GAME_WIN : {
            drawEndScreen(draw_list, childMin, childMax, true);
            break;
        }
        default :{
            std::cout<<"Unknown screen to show"<<std::endl;
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
    pl.name = name;
    pl.color = color;
    pl.wpn = Weapon(0);
    pl.timer_atk = -1;
    opponents.insert(std::make_pair(name, pl));
    this->bufferOnReceipt.addClient(pl);
}

auto form1 = [](float v) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", v);
    return std::string(buf);
};

void ClientUI::drawSelectionScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max) {
    ImVec2 size = ImVec2(max.x - min.x, max.y - min.y);
    float zoneHeight = size.y / 5.0f;

    ImVec2 screenCenter = ImVec2(
    min.x + size.x * 0.5f,
    min.y + size.y * 0.5f
);

    // Zones
    ImVec2 z0_min = ImVec2(min.x, min.y);
    ImVec2 z1_min = ImVec2(min.x, min.y + zoneHeight);
    ImVec2 z2_min = ImVec2(min.x, min.y + zoneHeight * 2);
    ImVec2 z3_min = ImVec2(min.x, min.y + zoneHeight * 3);
    ImVec2 z4_min = ImVec2(min.x, min.y + zoneHeight * 4);

    // Persistant variables
    static char nameBuffer[31] = "";
    static int selectedPreset = -1;
    static int selectedColor = 0;
    static int selectedWeapon = 1;          // id weapon = [1 -> 5]


    // button color definition
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 130));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 180));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 200));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 255, 255, 130));

    // Colors
    static std::vector<ImU32> colors = {
        IM_COL32(255, 255, 255, 255),           // white
        IM_COL32(255, 0, 0, 255),               // red
        IM_COL32(0, 255, 0, 255),               // green

        IM_COL32(0, 0, 255, 255),               // blue
        IM_COL32(0, 127, 255, 255),             // light blue
        IM_COL32(127, 0, 255, 255),             // dark purple

        IM_COL32(127, 255, 0, 255),             // green
        IM_COL32(0, 255, 127, 255),             // lime
        IM_COL32(255, 127, 0, 255),             // orange

        IM_COL32(255, 0, 127, 255),             // pink
        IM_COL32(158, 72, 203, 255)             // pony purple
    };

    // Presets
    static std::vector<Preset> presets = {
        {"Client A", 1, 1},
        {"Client B", 2, 1},
        {"Twigg", 0, 10},
        {"Pony", 10, 11},
        {"Bescher", 7, 12}
    };

    // Weapon
    static Weapon demoWeapon;
    demoWeapon.applyID(selectedWeapon);

    float buttonSize = ImGui::GetFrameHeight();
    float spacing = ImGui::GetStyle().ItemSpacing.x;

    // ZONE 0 : TITLE
    {
        std::string title = "Sélection des joueurs";
        ImVec2 text_size = ImGui::CalcTextSize(title.c_str());

        draw_list->AddText(
            ImVec2(
                screenCenter.x - text_size.x * 0.5f,
                z0_min.y + (zoneHeight - text_size.y) * 0.5f
                ),
            IM_COL32(255,255,255,255),
            title.c_str()
        );
    }

    // ZONE 1 : NAME
    {
        // insertion zone
        float inputWidth = 200.f;
        ImGui::SetCursorScreenPos(ImVec2(
            screenCenter.x - inputWidth * 0.5f,
            z1_min.y + 10
        ));

        ImGui::PushItemWidth(inputWidth);
        if (ImGui::InputText("##name", nameBuffer, 31)) {
            selectedPreset = -1;
        }
        ImGui::PopItemWidth();

        // Buttons
        float totalWidth = buttonSize * 2 + spacing;
        ImGui::SetCursorScreenPos(ImVec2(
            screenCenter.x - totalWidth * 0.5f,
            z1_min.y + 50
        ));
        if (ImGui::ArrowButton("##left_name", ImGuiDir_Left)) {
            selectedPreset = (selectedPreset - 1 + presets.size()) % presets.size();
            strcpy(nameBuffer, presets[selectedPreset].name.c_str());
            selectedColor = presets[selectedPreset].color;
            selectedWeapon = presets[selectedPreset].weapon;
        }

        ImGui::SameLine(0, spacing);

        if (ImGui::ArrowButton("##right_name", ImGuiDir_Right)) {
            selectedPreset = (selectedPreset + 1) % presets.size();
            strcpy(nameBuffer, presets[selectedPreset].name.c_str());
            selectedColor = presets[selectedPreset].color;
            selectedWeapon = presets[selectedPreset].weapon;
        }
    }

    // ZONE 2 : COLOR
    {
        // color circle
        float radius = 20.f;
        ImVec2 center = ImVec2(
            screenCenter.x,
            z2_min.y + zoneHeight * 0.4f
        );
        draw_list->AddCircleFilled(center, radius, colors[selectedColor]);

        // Buttons
        float totalWidth = buttonSize * 2 + spacing;
        ImGui::SetCursorScreenPos(ImVec2(
            screenCenter.x - totalWidth * 0.5f,
            z2_min.y + zoneHeight * 0.7f
        ));
        if (ImGui::ArrowButton("##left_color", ImGuiDir_Left)) {
            selectedColor = (selectedColor - 1 + colors.size()) % colors.size();
        }

        ImGui::SameLine(0, spacing);

        if (ImGui::ArrowButton("##right_color", ImGuiDir_Right)) {
            selectedColor = (selectedColor + 1) % colors.size();
        }
    }

    // ZONE 3 : WEAPON
    {
        float halfWidth = 80.f;

        ImU32 weaponColor = colors[selectedColor];

        float h = demoWeapon.getHeight();
        float w = demoWeapon.getWidth();

        // ZONE 3 - 1 : WPN DRAW

        ImVec2 leftCenter = ImVec2(
            screenCenter.x - halfWidth,
            z3_min.y + zoneHeight * 0.45f
        );

        switch (demoWeapon.getType()) {

            case Weapons::TRIANGLE:
            {
                ImVec2 p1 = ImVec2(leftCenter.x, leftCenter.y - h * 0.5f);
                ImVec2 p2 = ImVec2(leftCenter.x - w * 0.5f, leftCenter.y + h * 0.5f);
                ImVec2 p3 = ImVec2(leftCenter.x + w * 0.5f, leftCenter.y + h * 0.5f);

                draw_list->AddTriangleFilled(p1, p2, p3, weaponColor);
                break;
            }

            case Weapons::RECTANGLE:
            {
                ImVec2 minRect = ImVec2(leftCenter.x - w * 0.5f, leftCenter.y - h * 0.5f);
                ImVec2 maxRect = ImVec2(leftCenter.x + w * 0.5f, leftCenter.y + h * 0.5f);

                draw_list->AddRectFilled(minRect, maxRect, weaponColor);
                break;
            }
            case Weapons::CIRCLE: {
                draw_list->AddCircleFilled(leftCenter, h * 0.5f, weaponColor);
                break;
            }
            default:
            {
                draw_list->AddCircleFilled(leftCenter, h * 0.5f, weaponColor);
                break;
            }
        }

        // ZONE 3 - 2: INFOS

        // Selection type
        std::string type;
        switch (demoWeapon.getType()) {
            case Weapons::TRIANGLE: type = "Triangle"; break;
            case Weapons::RECTANGLE: type = "Rectangle"; break;
            default: type = "Unknown"; break;
        }

        std::string info =
            "Range: " + form1(demoWeapon.getRange()) +
            " | Damage: " + form1(demoWeapon.getDamage()) +
            "\nReload: " + form1(demoWeapon.getReload()) +
            " | Type: " + type;


        ImVec2 text_size = ImGui::CalcTextSize(info.c_str());

        ImVec2 rightPos = ImVec2(
            screenCenter.x + halfWidth - text_size.x * 0.5f,
            z3_min.y + zoneHeight * 0.25f
        );

        draw_list->AddText(rightPos, IM_COL32(220,220,220,255), info.c_str());

        // Buttons
        float totalWidth = buttonSize * 2 + spacing;
        ImGui::SetCursorScreenPos(ImVec2(
            screenCenter.x - totalWidth * 0.5f,
            z3_min.y + zoneHeight * 0.8f
        ));


        const int weaponCount = 5;

        if (ImGui::ArrowButton("##left_weapon", ImGuiDir_Left)) {
            selectedWeapon--;
            if (selectedWeapon < 1) selectedWeapon = weaponCount;
        }

        ImGui::SameLine(0, spacing);

        if (ImGui::ArrowButton("##right_weapon", ImGuiDir_Right)) {
            selectedWeapon++;
            if (selectedWeapon > weaponCount) selectedWeapon = 1;
        }
    }

    // ZONE 4 : CONFIRM
    {
        float buttonWidth = 120.f;
        ImGui::SetCursorScreenPos(ImVec2(
            screenCenter.x - buttonWidth * 0.5f,
            z4_min.y + zoneHeight * 0.3f
        ));

        if (ImGui::Button("CONFIRMER", ImVec2(buttonWidth, 40))) {
            std::string finalName;
            if (strlen(nameBuffer) == 0) {
                finalName = "Client-" + std::to_string(this->getPlayer().port);
            } else {
                finalName = nameBuffer;
            }

            // TODO : make this so the player is
            std::cout << "=>PLAYER CONFIG<=" << std::endl;
            std::cout << "Name: " << finalName << std::endl;
            std::cout << "Color ID: " << selectedColor << std::endl;
            std::cout << "Weapon ID: " << selectedWeapon << std::endl;
        }
    }
    ImGui::PopStyleColor(4);
}

void ClientUI::drawLoadingScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max) {
        ImFont* font = ImGui::GetFont();
        auto player = getPlayer();

        float width  = max.x - min.x;

        ImVec2 center = {
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f
        };

        // BACKGROUND
        draw_list->AddRectFilled(min, max, IM_COL32(20, 20, 20, 255));

        // LOADER
        {
            static float t = 0.0f;
            t += ImGui::GetIO().DeltaTime * 3.0f;

            float radius = 25.0f;
            int segments = 30;

            float start = t;
            float end   = t + std::numbers::pi * 1.5f;

            draw_list->PathClear();

            for (int i = 0; i < segments; i++) {
                float a = start + (end - start) * ((float)i / segments);

                ImVec2 p = {
                    center.x + cosf(a) * radius,
                    center.y - 40 + sinf(a) * radius
                };

                draw_list->PathLineTo(p);
            }
            draw_list->PathStroke(
                IM_COL32(255,255,255,180),
                false,
                4.0f
            );
        }

        // LOADING TEXT
        {
            const char* txt;
            switch (player.status) {
                case Status::WAITING_FOR_INIT : {
                    txt = "Loading... \n Sending your data to the server...";
                    break;
                }
                case Status::WAITING_FOR_OPPONENTS  : {
                    txt = "Loading... \n Waiting for your opponents...";
                    break;
                }
                case Status::READY_TO_START: {
                    txt = "Loading... \n Ready to start...";
                    break;
                }
                default : {
                    txt = "Loading...";
                    break;
                }
            }

            float size = 20.0f;

            ImVec2 dim = font->CalcTextSizeA(size, FLT_MAX, 0.0f, txt);

            ImVec2 pos = {
                center.x - dim.x * 0.5f,
                center.y + 10
            };

            draw_list->AddText(
                font,
                size,
                pos,
                IM_COL32(220, 220, 220, 255),
                txt
            );
        }

        // RANDOM PRAISE
        {
            static std::vector<std::string> messages = {
                "Achat de la RAM...",
                "Si le jeu charge pas, check les config en haut",
                "Chargement des skills... (introuvables)",
            };

            static int selected = player.port % messages.size();
            std::string msg = messages[selected];

            float msg_size = 18.0f;

            float wrap_width = width * 0.8f;

            ImVec2 text_dim = font->CalcTextSizeA(
                msg_size,
                wrap_width,
                0.0f,
                msg.c_str()
            );

            ImVec2 msg_center = {
                center.x,
                center.y + 100
            };

            ImVec2 msg_pos = {
                msg_center.x - text_dim.x * 0.5f,
                msg_center.y - text_dim.y * 0.5f
            };

            draw_list->AddText(
                font,
                msg_size,
                msg_pos,
                IM_COL32(180, 180, 180, 255),
                msg.c_str(),
                nullptr,
                wrap_width
            );
        }
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
void ClientUI::drawWeapon(Player player, ImDrawList* draw_list, ImVec2 pl_position, float scale) {
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

        case Weapons::SHIELD: {
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
        static int selected = this->getPlayer().port % messages.size();
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

    // BUTTON
    {
        float btn_font_size = 22.0f;
        int nb_buttons = 3;
        float gap = 15.0f;

        // Buttons width
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
                std::cout<<"CLICK ON "<< btn_text <<std::endl;
                screenToShow = Screens::LOADING_SCREEN;
                setStatus(Status::WAITING_FOR_INIT);

                // TODO : sending new players packet to the server with the actual player values
                // TODO : protecting the server so he don't stop the game while playing on retry

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
                setLoop(false);
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
