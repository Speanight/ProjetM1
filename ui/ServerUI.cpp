#include "ServerUI.hpp"

ServerUI::ServerUI(std::chrono::time_point<std::chrono::steady_clock> clock) : Server(clock) {}

void ServerUI::addLine(const std::string& text, sf::Color color, sf::Text::Style style) {
    sf::Text line(font);
    line.setString(text);
    line.setCharacterSize(size);
    line.setFillColor(color);
    line.setStyle(style);
    line.setPosition(sf::Vector2f(position.x, position.y + lines.size() * (size + lineSpace)));
    lines.push_back(line);
}

void ServerUI::draw() {
    ImGui::BeginChild("Server", ImVec2(0, 0), true);
    ImGui::Text("SERVER");
    ImGui::Separator();
    ImGui::TextWrapped("Server text aaaaaaaaaaaaa");
    ImGui::EndChild();
}