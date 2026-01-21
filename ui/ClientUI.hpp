#ifndef PROJETM1_CLIENTUI_HPP
#define PROJETM1_CLIENTUI_HPP

#include <imgui.h>
#include <imgui-SFML.h>
#include "../communication/Client.hpp"

class ClientUI : public Client {
private:

public:
    ClientUI(std::chrono::time_point<std::chrono::steady_clock> clock, std::string name, char *title);
    void drawGame();
    void drawConfig();
};


#endif //PROJETM1_CLIENTUI_HPP
