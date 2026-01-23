#ifndef PROJETM1_SERVERUI_HPP
#define PROJETM1_SERVERUI_HPP

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <imgui.h>
#include <ranges>
#include <utility>

#include "../Utils.hpp"

struct ConsoleLine {
    sf::Color color;
    std::string text;
};

class ServerUI {
private:
    std::vector<ConsoleLine> lines;
    sf::Font font;
    int lineSpace = 5;
    sf::Vector2f position;
    int size = 32;
public:
    ServerUI();
    void addLine(std::string text, sf::Color color = sf::Color::White);
    void draw();
};

#endif //PROJETM1_SERVERUI_HPP
