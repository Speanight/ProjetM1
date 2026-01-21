#ifndef PROJETM1_SERVERUI_HPP
#define PROJETM1_SERVERUI_HPP

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <imgui.h>

#include "../communication/Server.hpp"

class ServerUI : public Server {
private:
    std::vector<sf::Text> lines;
    sf::Font font;
    int lineSpace = 5;
    sf::Vector2f position;
    int size = 32;
public:
    ServerUI(std::chrono::time_point<std::chrono::steady_clock> clock);
    void addLine(const std::string& text, sf::Color color = sf::Color::White, sf::Text::Style style = sf::Text::Regular);
    void draw();
};


#endif //PROJETM1_SERVERUI_HPP
