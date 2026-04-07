#ifndef PROJETM1_SERVERUI_HPP
#define PROJETM1_SERVERUI_HPP

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <imgui.h>
#include <ranges>
#include <utility>
#include "../src/implot.h"
#include "../src/implot_internal.h"
#include "../game/gameUtils.hpp"
#include <list>

#include "../ui/Console.hpp"
#include "../Utils.hpp"

struct ConsoleLine {
    sf::Color color;
    std::string text;
};

class ServerUI {
private:
    std::vector<ConsoleLine> lines;
    sf::Font font;
    sf::Vector2f position;
    int size = 32;
    bool pauseConsole = false;

    std::mutex m;

    // Graphs
    int lastTimestamp;

protected:
    // Data (graphs)
    std::unordered_map<std::string, std::vector<float>> data;
    bool rewind = false; // Compensation method.
    Console& console;

    std::unordered_map<unsigned short, Player> clients;
    int mapID;

public:
    ServerUI(Console& console);

    // Graph 1
    void addLine(std::string text, sf::Color color = sf::Color::White);
    void addLine(int timestamp, std::string from, std::string to, std::string details, sf::Color color = sf::Color::White);
    void addToGraph(int timestamp, const std::string& from, const std::string& to);
    void draw();

    // Graph 2
    void setPlayer(unsigned short id, Player player);
    void updateClient(unsigned short id, State s);
    void setMapID(int mapID);
    void drawGame();

    // Graph 3
    void addToData(const std::string& to);
    void removeToData(const std::string& to);
};

#endif //PROJETM1_SERVERUI_HPP
