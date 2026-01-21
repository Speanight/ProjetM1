#include "ServerUI.hpp"

ServerUI::ServerUI(const std::string& fontPath, const sf::Vector2f& pos) : position(pos) {
    if (!font.openFromFile(fontPath)) {
        // TODO: Error handler
    }
}

ServerUI::ServerUI() = default;

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
    ImGui::TextWrapped("Server");
    ImGui::EndChild();
}