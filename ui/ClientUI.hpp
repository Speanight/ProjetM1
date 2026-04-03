#ifndef PROJETM1_CLIENTUI_HPP
#define PROJETM1_CLIENTUI_HPP

#include <imgui.h>
#include <imgui-SFML.h>
#include "../communication/Client.hpp"
#include "../game/gameUtils.hpp"
#include "../game/Player.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/Color.hpp>
#include <algorithm>

struct UISelect {
    char nameBuffer[31] = "";
    int selectedPreset = -1;
    int selectedColor = 0;
    int selectedWeapon = 1;

    bool allow_moove;

    int selectedZone = 0; // 0=NAME, 1=COLOR, 2=WEAPON, 3=CONFIRM
    int subSelect = 0;    // 0=left, 1=right (ou bouton unique pour confirm)
};

class ClientUI : public Client {
protected :
    short screenToShow;
    int waitRetry = 0;
public:
    ClientUI(sf::Clock& clock, Console& console, std::string name, short controller = -1, sf::Color color = sf::Color::Red);

    void addOpponent(const std::string& name, sf::Color color);

    // Draw functions
    void drawGame();
    void drawConfig();

    void drawSelectionScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max);

    void drawLoadingScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max);

    void drawEndScreen(ImDrawList* draw_list, ImVec2 min, ImVec2 max, bool victory);
};


#endif //PROJETM1_CLIENTUI_HPP
