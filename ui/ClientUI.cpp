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
    // DRAW
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // TODO : => mooving this switch case in an other loop ? <=
    if(getLoop() == false) {
        screenToShow = Screens::PLAYER_SELECT;
    }
    else {
        auto player = getPlayer();
        // Switching the screen to show depending on the player status
        switch (player.getStatus()) {
            case Status::WAITING_FOR_INIT or Status::WAITING_FOR_OPPONENTS or Status::READY_TO_START: {
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
                std::cout<<"Unknown player status to show " << player.getStatus() <<std::endl;
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
    pl.setName(name);
    pl.setColor(color);
    opponents.insert(std::make_pair(name, pl));
    this->bufferOnReceipt.addClient(pl);
}

// put the value to print to 2 digits
auto form1 = [](float v) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", v);
    return std::string(buf);
};

void ClientUI::drawSelectionScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max) {
    float window_size = max.x - min.x;
    float scale = window_size / Const::MAP_SIZE_X;

    ImVec2 size = ImVec2(max.x - min.x, max.y - min.y);
    float zoneHeight = size.y / 5.0f;

    ImVec2 screenCenter = ImVec2(
    min.x + size.x * 0.5f,
    min.y + size.y * 0.5f
    );

    std::string port = std::to_string(int(getPlayer().getPort()));

    // Zones
    ImVec2 z0_min = ImVec2(min.x, min.y);
    ImVec2 z1_min = ImVec2(min.x, min.y + zoneHeight);
    ImVec2 z2_min = ImVec2(min.x, min.y + zoneHeight * 2);
    ImVec2 z3_min = ImVec2(min.x, min.y + zoneHeight * 3);
    ImVec2 z4_min = ImVec2(min.x, min.y + zoneHeight * 4);

    // Persistant variables
    static std::unordered_map<int, UISelect> selects;
    UISelect& select = selects[getPlayer().getPort()];

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
    demoWeapon.applyID(select.selectedWeapon);

    float buttonSize = ImGui::GetFrameHeight();
    float spacing = ImGui::GetStyle().ItemSpacing.x;

    // ZONE 0 : TITLE
    {
        std::string title = "Sélection des joueurs";
        ImVec2 text_size = ImGui::CalcTextSize(title.c_str());

        draw_list->AddText(
            ImVec2(
                screenCenter.x - text_size.x * 0.5f * scale,
                z0_min.y + (zoneHeight - text_size.y) * 0.5f * scale
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
            screenCenter.x - inputWidth * 0.5f * scale,
            z1_min.y *scale + 10
        ));

        ImGui::PushItemWidth(inputWidth);
        if (ImGui::InputText(("##name"+port).c_str(), select.nameBuffer, 31)) {
            select.selectedPreset = -1;
        }
        ImGui::PopItemWidth();

        // Buttons
        float totalWidth = buttonSize * 2 + spacing;
        ImGui::SetCursorScreenPos(ImVec2(
            screenCenter.x - totalWidth * 0.5f * scale,
            z1_min.y * scale + 50
        ));
        if (ImGui::ArrowButton(("##left_name"+port).c_str(), ImGuiDir_Left)) {
            select.selectedPreset = (select.selectedPreset - 1 + presets.size()) % presets.size();
            strcpy(select.nameBuffer, presets[select.selectedPreset].name.c_str());
            select.selectedColor = presets[select.selectedPreset].color;
            select.selectedWeapon = presets[select.selectedPreset].weapon;
        }

        ImGui::SameLine(0, spacing);

        if (ImGui::ArrowButton(("##right_name"+port).c_str(), ImGuiDir_Right)) {
            select.selectedPreset = (select.selectedPreset + 1) % presets.size();
            strcpy(select.nameBuffer, presets[select.selectedPreset].name.c_str());
            select.selectedColor = presets[select.selectedPreset].color;
            select.selectedWeapon = presets[select.selectedPreset].weapon;
        }
    }

    // ZONE 2 : COLOR
    {
        // color circle
        float radius = Const::PLAYER_SIZE*scale;
        ImVec2 center = ImVec2(
            screenCenter.x,
            z2_min.y + zoneHeight * 0.4f
        );
        draw_list->AddCircleFilled(center, radius, colors[select.selectedColor]);

        // Buttons
        float totalWidth = buttonSize * 2 + spacing;
        ImGui::SetCursorScreenPos(ImVec2(
            screenCenter.x - totalWidth * 0.5f,
            z2_min.y + zoneHeight * 0.7f
        ));
        if (ImGui::ArrowButton("##left_color", ImGuiDir_Left)) {
            select.selectedColor = (select.selectedColor - 1 + colors.size()) % colors.size();
        }

        ImGui::SameLine(0, spacing);

        if (ImGui::ArrowButton("##right_color", ImGuiDir_Right)) {
            select.selectedColor = (select.selectedColor + 1) % colors.size();
        }
    }

    // ZONE 3 : WEAPON
    {
        float halfWidth = 80.f;

        ImU32 weaponColor = colors[select.selectedColor];

        float h = demoWeapon.getHeight()*scale;
        float w = demoWeapon.getWidth()*scale;

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
            screenCenter.x + halfWidth - text_size.x * scale * 0.5f,
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
            select.selectedWeapon--;
            if (select.selectedWeapon < 1) select.selectedWeapon = weaponCount;
        }

        ImGui::SameLine(0, spacing);

        if (ImGui::ArrowButton("##right_weapon", ImGuiDir_Right)) {
            select.selectedWeapon++;
            if (select.selectedWeapon > weaponCount) select.selectedWeapon = 1;
        }
    }

    // ZONE 4 : CONFIRM
    {
        float buttonWidth = 120.f*scale;
        ImGui::SetCursorScreenPos(ImVec2(
            screenCenter.x - buttonWidth * 0.5f,
            z4_min.y + zoneHeight * 0.3f
        ));

        if (ImGui::Button("CONFIRMER", ImVec2(buttonWidth, 40))) {
            std::string finalName;
            if (strlen(select.nameBuffer) == 0) {
                finalName = "Client-" + std::to_string(this->getPlayer().getPort());
            } else {
                finalName = select.nameBuffer;
            }

            // TODO : make this so the player is sent to the server
            std::cout << "=>PLAYER CONFIG<=" << std::endl;
            std::cout << "Name: " << finalName << std::endl;
            std::cout << "Color ID: " << select.selectedColor << std::endl;
            std::cout << "Weapon ID: " << select.selectedWeapon << std::endl;

            // deleting memory of the selector
            selects.erase(this->getPlayer().getPort());
            // demoWeapon is automaticly destroyed at the end of the program

        }
    }
    ImGui::PopStyleColor(4);
}

void ClientUI::drawLoadingScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max) {
    float window_size = max.x - min.x;
    float scale = window_size / Const::MAP_SIZE_X;

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

        float radius = 25.0f*scale;
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
        switch (player.getStatus()) {
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

        float size = 20.0f*scale;

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
            "Si le jeu charge pas, check les config en haut PTDR",
            "Allez chatGpt, fait moi le jeu",
            "Lancement de missiles nucléaires sur washingtown...",
            "Attente de rachat par EA..."
        };

        static int selected = player.getPort() % messages.size();
        std::string msg = messages[selected];

        float msg_size = 18.0f*scale;

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
    static std::vector<std::string> map_link = {
        "../font/map/dalle1.jpg",
        "../font/map/grass1.jpg",
        "../font/map/grass2.jpg",
    };

    int selected = getMapID() % map_link.size();
    std::string map = map_link[selected];

    static sf::Texture Texture;
    if(Texture.loadFromFile(map)) {
        draw_list->AddImage(
            (ImTextureID)Texture.getNativeHandle(),
            ImVec2(min.x, min.y),
            ImVec2(max.x, max.y)
        );
    }
    else {
        std::cout<<"Map not loaded"<<std::endl;
    }

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
        case Weapons::TRIANGLE  : {
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

            ImVec2 dir = { cosf(angle), sinf(angle) };

            ImVec2 perp = { -dir.y, dir.x };

            ImVec2 center = {
                pl_position.x + dir.x * distance,
                pl_position.y + dir.y * distance
            };

            ImVec2 p1 = { center.x + perp.x * (width * 0.5f), center.y + perp.y * (width * 0.5f) };
            ImVec2 p2 = { center.x - perp.x * (width * 0.5f), center.y - perp.y * (width * 0.5f) };

            ImVec2 p3 = { p2.x + dir.x * height, p2.y + dir.y * height };
            ImVec2 p4 = { p1.x + dir.x * height, p1.y + dir.y * height };

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

void ClientUI::drawEndScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max, bool victory) {
    float window_size = max.x - min.x;
    float scale = window_size / Const::MAP_SIZE_X;

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
        float title_size = 50.0f*scale;

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

        float msg_size = 18.0f*scale;

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
        float btn_font_size = 20.0f*scale;
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
                setStatus(Status::WAITING_FOR_INIT);
                screenToShow = Screens::LOADING_SCREEN;

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