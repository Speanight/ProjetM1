#ifndef PROJETM1_CLIENTUI_HPP
#define PROJETM1_CLIENTUI_HPP

#include <imgui.h>
#include <imgui-SFML.h>
#include "../communication/Client.hpp"

class ClientUI : public Client {
private:

protected:
    std::map<std::string, ClientUI> opponents;
public:
    ClientUI(sf::Clock clock, std::string name, sf::Color color = sf::Color::Red);

    // PlayerInit
    void drawInit(ImVec2 center, float radius, float speed, ImU32 color);

    void addOpponent(sf::Clock clock, const std::string& name, sf::Color color);

    void drawGame();
    void drawPlayer(ImDrawList* draw_list);
    void drawConfig();
};


#endif //PROJETM1_CLIENTUI_HPP
