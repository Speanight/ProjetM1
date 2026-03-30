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

    // Graphs
    int lastTimestamp;

protected:
    // Data (graphs)
    std::unordered_map<std::string, std::vector<float>> data;
    bool rewind = false; // Compensation method.
    Console console;

public:
    ServerUI();
    void addLine(std::string text, sf::Color color = sf::Color::White);
    void addLine(int timestamp, std::string from, std::string to, std::string details, sf::Color color = sf::Color::White);
    void addToGraph(int timestamp, const std::string& from, const std::string& to);
    void draw();

    void addToData(const std::string& to);
    void removeToData(const std::string& to);
};

#endif //PROJETM1_SERVERUI_HPP
