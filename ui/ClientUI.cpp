#include "ClientUI.hpp"

ClientUI::ClientUI(const sf::Clock clock, std::string name, sf::Color color) : Client(clock, name, color) {}
// ClientUI::ClientUI(const sf::Clock clock, std::string name, sf::Color color, float radius) : Client(clock, name, color, radius) {}

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

    // ========= INPUT =========
    ImVec2 dir = {0.f, 0.f};

    Input inputs = getInputs();

    dir.x += 1.f * inputs.getMovementX();
    dir.y += 1.f * inputs.getMovementY();

        // ========= DRAW =========

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        drawPlayer(draw_list, getPlayer(), childMin, childMax);

        for (auto & [name, other] : opponents) {
            drawPlayer(draw_list, other, childMin, childMax);
        }

    // PREDICTION
//    if (true) {
//        this->setPosition(smoothenDeplacement(getPosition(), dir, lastUpdate, clock.getElapsedTime().asMilliseconds()));
//    }

    lastUpdate = clock.getElapsedTime().asMilliseconds();
    ImGui::EndChild();
}


void ClientUI::addOpponent(const std::string& name, sf::Color color) {
    opponents.insert(std::make_pair(name, Player(0,name, color, Position())));
}

void ClientUI::drawConfig() {
    const char* title = getName().c_str();

    ImGui::BeginChild(title, ImVec2(0, 100), true); // hauteur FIXE
    ImGui::Text("%s", title);
    ImGui::Separator();

    int packetLoss = getPacketLoss();
    int ping = getPing();

    ImGui::SliderInt("Packet loss", &packetLoss, 0, 100);
    ImGui::InputInt("Ping", &ping);

    setPacketLoss(packetLoss);
    setPing(ping);

    ImGui::EndChild();
}