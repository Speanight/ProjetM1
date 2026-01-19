#include "Console.hpp"
Console::Console(const std::string& fontPath, const sf::Vector2f& pos) : position(pos) {
    if (!font.openFromFile(fontPath)) {
        // TODO: Error handler
    }
}

void Console::addLine(const std::string& text,sf::Color color, sf::Text::Style style) {
    sf::Text line(font);
    line.setString(text);
    line.setCharacterSize(size);
    line.setFillColor(color);
    line.setStyle(style);
    line.setPosition(sf::Vector2f(position.x, position.y + lines.size() * (size + lineSpace)));
    lines.push_back(line);
}

void Console::draw(sf::RenderWindow& window) {
    for (auto& line : lines) {
        window.draw(line);
    }
}