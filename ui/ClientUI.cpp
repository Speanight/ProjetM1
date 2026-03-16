#include "ClientUI.hpp"

#include <algorithm>

ClientUI::ClientUI(const sf::Clock clock, std::string name, sf::Color color) : Client(clock, name, color) {}

void ClientUI::drawGame() { // Game space
    const char* title = getName().c_str();

    // calculating the window so it's always a square
    // Max available size from parent window
    ImVec2 avail = ImGui::GetContentRegionAvail();

    // game size
    float size = std::max(std::min(avail.x, avail.y - 300.f), 400.f); //TODO : replace the 500.f by a constant that is the size of the server UI

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


    drawPlayer(draw_list,getTruePlayer() , childMin, childMax, clock);


    for (auto & [name, other] : opponents) {
        drawPlayer(draw_list, other, childMin, childMax, clock);
    }
    ImGui::EndChild();
}


void ClientUI::addOpponent(const std::string& name, sf::Color color) {
    Player pl;
    pl.name = name;
    pl.color = color;
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

    int packetLoss = getPacketLoss();
    int ping = getPing();
    std::array<bool,3> compensations = getCompensations();

    ImGui::SliderInt("Packet loss", &packetLoss, 0, 100);
    ImGui::InputInt("Ping", &ping);

    ImGui::Checkbox("Interpolation", &compensations[Compensation::INTERPOLATION]);
    ImGui::SameLine();
    ImGui::Checkbox("Prediction", &compensations[Compensation::PREDICTION]);
    ImGui::SameLine();
    ImGui::Checkbox("Reconciliation", &compensations[Compensation::RECONCILIATION]);

    setPacketLoss(packetLoss);
    setPing(ping);

    if (!compensations[Compensation::PREDICTION] and compensations[Compensation::RECONCILIATION]) {
        compensations[Compensation::PREDICTION] = true;
    }

    setCompensations(compensations);

    ImGui::EndChild();
}