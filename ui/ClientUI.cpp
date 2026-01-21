#include "ClientUI.hpp"

ClientUI::ClientUI(std::chrono::time_point<std::chrono::steady_clock> clock, std::string name) : Client(clock, name) {}

void ClientUI::drawGame() {
    const char* title = getName().c_str();

    ImGui::BeginChild(title, ImVec2(0, 400), true);
    ImGui::Text("%s", title);
    ImGui::Separator();
    ImGui::Text("GAME COMING BEFORE GTA VI ! DW ! ");
    ImGui::EndChild();
}

void ClientUI::drawConfig() {
    bool changed = false;

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