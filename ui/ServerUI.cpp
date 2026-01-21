#include "ServerUI.hpp"

#include <utility>

// TODO: Invert - server HAS A UI, not UI having a server

ServerUI::ServerUI() {}

void ServerUI::addLine(std::string text, sf::Color color) {
    ConsoleLine cl;
    cl.color = color;
    cl.text = std::move(text);
    lines.push_back(cl);
}

void ServerUI::draw() {
    ImGui::BeginChild("Server", ImVec2(0, 0), true);
    ImGui::Text("SERVER");
    ImGui::Separator();
    for (const auto& line : std::ranges::reverse_view(lines)) {
        ImGui::TextColored(line.color, "%s", line.text.c_str());
    }
    ImGui::EndChild();
}