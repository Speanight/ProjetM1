#ifndef PROJETM1_CONSOLE_HPP
#define PROJETM1_CONSOLE_HPP

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class Console {
private:
    std::vector<sf::Text> lines;
    sf::Font font;
    int lineSpace = 5;
    sf::Vector2f position;
    int size = 32;
public:
    Console(const std::string& fontPath, const sf::Vector2f& pos);
    void addLine(const std::string& text, sf::Color color = sf::Color::White, sf::Text::Style style = sf::Text::Regular);
    void draw(sf::RenderWindow& window);
};


#endif //PROJETM1_CONSOLE_HPP
