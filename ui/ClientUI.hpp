#ifndef PROJETM1_CLIENTUI_HPP
#define PROJETM1_CLIENTUI_HPP

#include <imgui.h>
#include <imgui-SFML.h>
#include "../communication/Client.hpp"
#include "../game/gameUtils.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/Color.hpp>

class ClientUI : public Client {
private:
    float lastUpdate;


protected:
public:
    ClientUI(sf::Clock clock, std::string name, sf::Color color = sf::Color::Red);

    void addOpponent(const std::string& name, sf::Color color);

    void drawGame();
    void drawConfig();
};


#endif //PROJETM1_CLIENTUI_HPP
