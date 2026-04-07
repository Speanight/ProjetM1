#include "ClientUI.hpp"

ClientUI::ClientUI(sf::Clock& clock, Console& console, std::string name, short controller, sf::Color color) :
Client(clock, console, name, controller, color) {}

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
    if(!getLoop()) {
        screenToShow = Screens::PLAYER_SELECT;
    }
    else {
        auto player = getPlayer();
        // Switching the screen to show depending on the player status
        switch (player.getStatus()) {
            case Status::WAITING_FOR_INIT:
            case Status::WAITING_FOR_OPPONENTS:
            case Status::READY_TO_START: {
                screenToShow = Screens::LOADING_SCREEN;
                break;
            }
            case Status::DONE: {
                screenToShow = Screens::GAME;
                break;
            }
            case Status::DEAD: {
                if (waitRetry == 0) {
                    waitRetry = getTick() + Const::WAIT_RETRY_TIME;
                }
                screenToShow = Screens::GAME_LOSE;
                break;
            }
            case Status::WIN: {
                if (waitRetry == 0) { waitRetry = getTick() + Const::WAIT_RETRY_TIME; }
                screenToShow = Screens::GAME_WIN;
                break;
            }
            case Status::END_R: {
                // screenToShow remain the same;
                break;
            }
            default: {
                std::cout << "Unknown player status to show: #" << player.getStatus() << std::endl;
                drawErrorScreen(draw_list, childMin, childMax);
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
            drawFightingScreen(draw_list, player, opponents, childMin, childMax, getMapID());
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
            drawErrorScreen(draw_list, childMin, childMax);
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
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
    ImGui::InputInt("Ping S -> C", &ping[RECEIVED]);
//    ImGui::SameLine();
    ImGui::InputInt("Ping C -> S", &ping[SENT]);
    ImGui::PopItemWidth();

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

/**
 * Make the floating values after the '.' 2 digit long
 * @param v : floating value
 * @return  : the floating value but in a string of macimum 2 digit
 */
std::string form1(float v) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", v);
    return std::string(buf);
};

/**
 * draw the screen to select the player we want (color, weapon, name ...)
 * @param draw_list : will contain all the element that will be print at the end of the program
 * @param min           : point at the top left of the screen to define the size
 * @param max           : point at the bottom right of the screen to define the size
 */
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

    // Player colors available
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
        IM_COL32(165, 71, 193, 255)             // pony purple
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

    // Inputs to navigate on the page using buttons
    Input input = getInputs(false, false);

    // INPUT HANDLING
    {
        // resseting the click lock when clicked
        if(!input.getAttack() && !input.getChangeWpn() &&
           input.getMovementX() == 0 && input.getMovementY() == 0) {
            select.allow_moove = true;
        }

        if(select.allow_moove) {
            if(input.getAttack() || input.getChangeWpn()) { // click on the arrow
                select.allow_moove = false;

                switch(select.selectedZone) {
                    case 0: {   // NAME
                        if(select.subSelect == 0) {
                            select.selectedPreset = (select.selectedPreset - 1 + presets.size()) % presets.size();
                        } else {
                            select.selectedPreset = (select.selectedPreset + 1) % presets.size();
                        }
                        strcpy(select.nameBuffer, presets[select.selectedPreset].name.c_str());
                        select.selectedColor = presets[select.selectedPreset].color;
                        select.selectedWeapon = presets[select.selectedPreset].weapon;
                        break;
                    }
                    case 1: {   // COLOR
                        if(select.subSelect == 0) {
                            select.selectedColor = (select.selectedColor - 1 + colors.size()) % colors.size();
                        } else {
                            select.selectedColor = (select.selectedColor + 1) % colors.size();
                        }
                        break;
                    }
                    case 2: {   // WEAPON
                        const int weaponCount = 8;
                        if(select.subSelect == 0) {
                            select.selectedWeapon--;
                            if (select.selectedWeapon < 1) select.selectedWeapon = weaponCount;
                        } else {
                            select.selectedWeapon++;
                            if (select.selectedWeapon > weaponCount) select.selectedWeapon = 1;
                        }
                        break;
                    }
                    case 3: {   // CONFIRM
                        std::string finalName;
                        if (strlen(select.nameBuffer) == 0) {
                            finalName = "Client-";
                        } else {
                            finalName = select.nameBuffer ;
                        }
                        finalName += std::to_string(this->getPlayer().getPort()); // security of 2 player taking the same role

                        Player pl = getPlayer();

                        pl.setName(finalName);
                        pl.setColor(convertImUToSfColor(colors[select.selectedColor]));
                        pl.setWeapons({Weapons::SHIELD, short(select.selectedWeapon)});
                        setPlayer(pl);
                        screenToShow = Screens::LOADING_SCREEN;
                        setStatus(Status::WAITING_FOR_INIT);
                        setLoop(true);

                        // deleting memory of the selector
                        selects.erase(this->getPlayer().getPort());
                        // demoWeapon is automaticly destroyed at the end of the program

                        selects.erase(this->getPlayer().getPort());
                        break;
                    }
                }
            }

            if(input.getMovementY() < 0) { // mooving up (selector on top)
                select.allow_moove = false;
                select.selectedZone--;
                if(select.selectedZone < 0) select.selectedZone = 3;
            }
            if(input.getMovementY() > 0) { // mooving down (selector bellow)
                select.allow_moove = false;
                select.selectedZone++;
                if(select.selectedZone > 3) select.selectedZone = 0;
            }
            if(input.getMovementX() < 0) { // mooving left (selecte left arrow)
                select.allow_moove = false;
                select.subSelect = 0;
            }
            if(input.getMovementX() > 0) { // mooving right (select right arrow)
                select.allow_moove = false;
                select.subSelect = 1;
            }
        }
    }
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

        ImVec2 leftBtn = ImVec2(
            screenCenter.x - totalWidth * 0.5f * scale,
            z1_min.y * scale + 50
            );

        ImVec2 rightBtn = ImVec2(
            leftBtn.x + buttonSize + spacing,
            leftBtn.y
            );

        // highligth zone
        if(select.selectedZone == 0) {
            ImVec2 target = (select.subSelect == 0) ? leftBtn : rightBtn;

            draw_list->AddRect(
                target,
                ImVec2(target.x + buttonSize, target.y + buttonSize),
                IM_COL32(255,255,0,255), 0, 0, 3.0f
                );
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

        ImVec2 leftBtn = ImVec2(
            screenCenter.x - totalWidth * 0.5f,
            z2_min.y + zoneHeight * 0.7f
            );

        ImVec2 rightBtn = ImVec2(
            leftBtn.x + buttonSize + spacing,
            leftBtn.y
            );

        // highlight
        if(select.selectedZone == 1) {
            ImVec2 target = (select.subSelect == 0) ? leftBtn : rightBtn;

            draw_list->AddRect(
                target,
                ImVec2(target.x + buttonSize, target.y + buttonSize),
                IM_COL32(255,255,0,255), 0, 0, 3.0f
                );
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
            case Weapons::TRIANGLE: {
                ImVec2 p1 = ImVec2(leftCenter.x, leftCenter.y - h * 0.5f);
                ImVec2 p2 = ImVec2(leftCenter.x - w * 0.5f, leftCenter.y + h * 0.5f);
                ImVec2 p3 = ImVec2(leftCenter.x + w * 0.5f, leftCenter.y + h * 0.5f);

                draw_list->AddTriangleFilled(p1, p2, p3, weaponColor);
                break;
            }
            case Weapons::CIRCLE: {
                float radius = h * 0.5f;

                draw_list->AddCircleFilled(
                    leftCenter,
                    radius,
                    weaponColor
                );
                break;
            }
            case Weapons::RECTANGLE: {
                ImVec2 minRect = ImVec2(leftCenter.x - w * 0.5f, leftCenter.y - h * 0.5f);
                ImVec2 maxRect = ImVec2(leftCenter.x + w * 0.5f, leftCenter.y + h * 0.5f);

                draw_list->AddRectFilled(minRect, maxRect, weaponColor);
                break;
            }
            default:{
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
            case Weapons::CIRCLE: type = "Circle"; break;
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

        // ZONE 3 - 3: BUTTONS

        // Buttons
        float totalWidth = buttonSize * 2 + spacing;
        ImGui::SetCursorScreenPos(ImVec2(
            screenCenter.x - totalWidth * 0.5f,
            z3_min.y + zoneHeight * 0.8f
        ));


        const int weaponCount = 8;

        if (ImGui::ArrowButton("##left_weapon", ImGuiDir_Left)) {
            select.selectedWeapon--;
            if (select.selectedWeapon < 1) select.selectedWeapon = weaponCount;
        }

        ImGui::SameLine(0, spacing);

        if (ImGui::ArrowButton("##right_weapon", ImGuiDir_Right)) {
            select.selectedWeapon++;
            if (select.selectedWeapon > weaponCount) select.selectedWeapon = 1;
        }
        ImVec2 leftBtn = ImVec2(
            screenCenter.x - totalWidth * 0.5f,
            z3_min.y + zoneHeight * 0.8f
            );

        ImVec2 rightBtn = ImVec2(
            leftBtn.x + buttonSize + spacing,
            leftBtn.y
            );

        // highlight
        if(select.selectedZone == 2) {
            ImVec2 target = (select.subSelect == 0) ? leftBtn : rightBtn;

            draw_list->AddRect(
                target,
                ImVec2(target.x + buttonSize, target.y + buttonSize),
                IM_COL32(255,255,0,255), 0, 0, 3.0f
                );
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
                finalName = "Client-";
            } else {
                finalName = select.nameBuffer ;
            }
            finalName += std::to_string(this->getPlayer().getPort()); // security of 2 player taking the same role

            Player pl = getPlayer();

            pl.setName(finalName);
            pl.setColor(convertImUToSfColor(colors[select.selectedColor]));
            pl.setWeapons({Weapons::SHIELD, short(select.selectedWeapon)});
            setPlayer(pl);
            screenToShow = Screens::LOADING_SCREEN;
            setStatus(Status::WAITING_FOR_INIT);
            setLoop(true);

            // deleting memory of the selector
            selects.erase(this->getPlayer().getPort());
            // demoWeapon is automaticly destroyed at the end of the program
        }

        // highlight
        if(select.selectedZone == 3) {
            draw_list->AddRect(
                ImVec2(screenCenter.x - buttonWidth * 0.5f, z4_min.y + zoneHeight * 0.3f),
                ImVec2(screenCenter.x + buttonWidth * 0.5f, z4_min.y + zoneHeight * 0.3f + 40),
                IM_COL32(255,255,0,255), 0, 0, 2.0f
                );
        }
    }
    ImGui::PopStyleColor(4);
}

/**
 * draw the loading screen
 * -> when the player is waiting for it's datas
 * -> when the player is waiting for the other oponents
 * -> when the player is waiting for the game to start
 * @param draw_list     : will contain all the element that will be print at the end of the program
 * @param min           : point at the top left of the screen to define the size
 * @param max           : point at the bottom right of the screen to define the size
 */
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

/**
 * draw the game over screen with the two buttons to retry or change the player
 * @param draw_list     : will contain all the element that will be print at the end of the program
 * @param min           : point at the top left of the screen to define the size
 * @param max           : point at the bottom right of the screen to define the size
 * @param victory       : print the screen depending on the victory or defeat of the player
 */
void ClientUI::drawEndScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max, bool victory) {
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 180));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 200));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 255, 255, 130));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

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
            "Wahhh gg bro"
        };
    }
    else {
        title = "DOMMAGE";
        messages = {
            "You can do it !*\n (*statement of friendship only, you may not be able to do it)",
            "Il a triché t'inquiète",
            "Mais je me fait big gank là c'est quoi ça !",
            "Blammez les JOUEURS pas le jeu...",
            "La prochaine fois c'est la bonne",
            "En vrai, dans la vrai vie il te bat pas t'inquiète",
            "T'est meilleur en partant de la fin, ça va ^^'"
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
        int nb_buttons = 2;     // todo : if we don't supress the menu button, then put it back to 3
        float gap = 15.0f;

        float total_width = max.x - min.x;
        float side_margin = 20.0f;

        float usable_width = total_width - (gap * (nb_buttons - 1)) - 2 * side_margin;
        float btn_width = usable_width / nb_buttons;

        float start_x = min.x + side_margin;

        float btn_height = 60.0f;
        ImVec2 btn_size = { btn_width, btn_height };

        float y = y_btn + (zone_h - btn_height) * 0.5f;

        // Handle inputs
        Input inputs;
        std::string retryText;
        if(getTick()>waitRetry) {       // 3sec wait before starting a new game
            retryText = "RETRY\n(press attack)";
            inputs = getInputs(false, false);
        }
        else {
            retryText = "wait " + form1((waitRetry-getTick())/100.f) + "s" ;
            inputs = getInputs(false, true);
        }

        // ===== RETRY =====
        {
            ImGui::SetCursorScreenPos({ start_x, y });

            ImVec2 btn_min = { start_x, y };

            ImGui::SetCursorScreenPos(btn_min);
            if (ImGui::Button(retryText.c_str(), btn_size) || inputs.getAttack()) {
                // std::cout << "CLICK ON RETRY" << std::endl;
                setStatus(Status::WAITING_FOR_INIT);
                setLoop(true);
                screenToShow = Screens::LOADING_SCREEN;
                waitRetry = 0;
            }
        }

        // ===== CHANGE PLAYER =====
        {
            ImGui::SetCursorScreenPos({ start_x + (btn_width + gap), y });

            if (ImGui::Button("CHANGE PLAYER\n(press change wpn)", btn_size)|| inputs.getChangeWpn()) {
                setLoop(false);
                waitRetry = 0;
                std::cout << "CLICK ON CHANGE PLAYER" << std::endl;
            }
        }

        // ===== MENU =====     // TODO : delete the menu button ?
        /*{
            ImGui::SetCursorScreenPos({ start_x + 2 * (btn_width + gap), y });

            ImGui::PushStyleColor(ImGuiCol_Button,        IM_COL32(150, 0, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(180, 50, 50, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  IM_COL32(120, 0, 0, 255));

            if (ImGui::Button("MENU", btn_size)) {
                waitRetry = 0;
                std::cout << "CLICK ON MENU" << std::endl;
            }

            ImGui::PopStyleColor(3);
        }*/
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
}