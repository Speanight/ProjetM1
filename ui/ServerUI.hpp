#ifndef PROJETM1_SERVERUI_HPP
#define PROJETM1_SERVERUI_HPP

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <imgui.h>

class ServerUI {
private:
    std::vector<sf::Text> lines;
    sf::Font font;
    int lineSpace = 5;
    sf::Vector2f position;
    int size = 32;
public:
    ServerUI();
    ServerUI(const std::string& fontPath, const sf::Vector2f& pos);
    void addLine(const std::string& text, sf::Color color = sf::Color::White, sf::Text::Style style = sf::Text::Regular);
    void draw();
};


#endif //PROJETM1_SERVERUI_HPP
