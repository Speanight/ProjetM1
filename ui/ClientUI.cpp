#include "ClientUI.hpp"

#include <algorithm>

ClientUI::ClientUI(const sf::Clock clock, std::string name, short controller, sf::Color color) : Client(clock, name, controller, color) {}

void ClientUI::drawGame() { // Game space
    const char* title = getName().c_str();

    // calculating the window so it's always a square
    // Max available size from parent window
    ImVec2 avail = ImGui::GetContentRegionAvail();

    // game size
    float size = std::max(std::min(avail.x, avail.y - 300.f), 400.f); //TODO : replace the 300.f by a constant that is the size of the server UI

    // minimal game size
    size = std::max(size, 400.f);

    // center horizontally only (top aligned)
    float offsetX = (avail.x - size) * 0.5f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
    // no Y offset -> stays at top

    // insert the game space
    ImGui::BeginChild(title, ImVec2(size, size), true);

    ImVec2 childMin = ImGui::GetWindowPos();
    ImVec2 childMax = {
        childMin.x + size,
        childMin.y + size
    };
    // ========= DRAW =========
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto player = getPlayer();
    switch (player.status) {
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
            drawEndScreen(draw_list, player, childMin, childMax, false);
            break;
        }
        case Status::WIN : {
            // TODO [delete me in future merge] - will be triggered with the "WIN" status
            drawEndScreen(draw_list, player, childMin, childMax, true);
            break;
        }
        default         : {
            drawErrorScreen(draw_list, player, childMin, childMax);
            break;
        }
    }

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

/**
 * Draws the configuration that shows at the top of the player's screen. It is used to
 * change ping and packet loss values, as well as change compensation enabled/disabled.
 */
void ClientUI::drawConfig() {
    const char* title = getName().c_str();

    ImGui::BeginChild(title, ImVec2(0, 100), true); // hauteur FIXE
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