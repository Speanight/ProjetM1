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
protected :
    short screenToShow;
public:
    ClientUI(sf::Clock clock, std::string name, short controller = -1, sf::Color color = sf::Color::Red);
    // ClientUI(sf::Clock clock, std::string name, sf::Color color = sf::Color::Red, float radius = 0);

    void addOpponent(const std::string& name, sf::Color color);

    // Draw functions
    void drawGame();
    void drawConfig();

    void drawSelectionScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max);

    void drawLoadingScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max);

    void drawFightingScreen(ImDrawList* draw_list, const Player player, std::map<std::string, Player> opponents, ImVec2 min, ImVec2 max);
    void drawPlayer(ImDrawList* draw_list, const Player player, ImVec2 min, ImVec2 max);
    void drawWeapon(Player player, ImDrawList* draw_list, ImVec2 pl_position, float scale);

    void drawEndScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max, bool victory);
};


#endif //PROJETM1_CLIENTUI_HPP
