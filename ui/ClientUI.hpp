#ifndef PROJETM1_CLIENTUI_HPP
#define PROJETM1_CLIENTUI_HPP

#include <imgui.h>
#include <imgui-SFML.h>
#include "../communication/Client.hpp"

class ClientUI : public Client {
private:

public:
    ClientUI(const sf::Clock clock, std::string name);

    void drawGamePl1();
    void drawGamePl2();
    void drawConfig();
};


#endif //PROJETM1_CLIENTUI_HPP
