#include "ClientUI.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "../game/Player.hpp"

ClientUI::ClientUI(const sf::Clock clock, std::string name) : Client(clock, name) {}


void ClientUI::drawGamePl1() { //Player 1 Game Space
    const char* title = getName().c_str();

    ImGui::BeginChild(title, ImVec2(0, 400), true);

    ImVec2 childMin = ImGui::GetWindowPos();
    ImVec2 childMax = {
        childMin.x + ImGui::GetWindowSize().x,
        childMin.y + ImGui::GetWindowSize().y
    };

    static sf::Clock deltaClock;
    float deltaTime = deltaClock.restart().asSeconds();

    ImVec2 childPos  = ImGui::GetWindowPos();
    ImVec2 childSize = ImGui::GetWindowSize();

    static Player player1(20.f,
        { childPos.x + childSize.x / 3.f,
          childPos.y + childSize.y / 2.f },
        IM_COL32(0,255,0,255));

    static Player player2(20.f,
        { childPos.x + 2.f * childSize.x / 3.f,
          childPos.y + childSize.y / 2.f },
        IM_COL32(255,0,0,255));

    // ========= INPUT =========
    ImVec2 dir = {0.f, 0.f};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)){dir.y -= 1.f;}
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)){dir.y += 1.f;}
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)){dir.x -= 1.f;}
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)){dir.x += 1.f;}

    // ========= UPDATE =========
    player1.move(dir, deltaTime);

    player1.clampToChild(childMin, childMax);
    player2.clampToChild(childMin, childMax);

    player1.resolveCollision(player2);

    // ========= DRAW =========
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    player1.draw(draw_list);
    player2.draw(draw_list);

    ImGui::EndChild();

}

void ClientUI::drawGamePl2() { //Player 1 Game Space
    const char* title = getName().c_str();

    ImGui::BeginChild(title, ImVec2(0, 400), true);

    ImVec2 childMin = ImGui::GetWindowPos();
    ImVec2 childMax = {
        childMin.x + ImGui::GetWindowSize().x,
        childMin.y + ImGui::GetWindowSize().y
    };

    static sf::Clock deltaClock;
    float deltaTime = deltaClock.restart().asSeconds();

    ImVec2 childPos  = ImGui::GetWindowPos();
    ImVec2 childSize = ImGui::GetWindowSize();

    static Player player1(20.f,
        { childPos.x + childSize.x / 3.f,
          childPos.y + childSize.y / 2.f },
        IM_COL32(0,255,0,255));

    static Player player2(20.f,
        { childPos.x + 2.f * childSize.x / 3.f,
          childPos.y + childSize.y / 2.f },
        IM_COL32(255,0,0,255));

    // ========= INPUT =========
    ImVec2 dir = {0.f, 0.f};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    dir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  dir.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dir.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dir.x += 1.f;

    // ========= UPDATE =========
    player2.move(dir, deltaTime);

    player2.clampToChild(childMin, childMax);
    player1.clampToChild(childMin, childMax);

    player2.resolveCollision(player1);

    // ========= DRAW =========
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    player2.draw(draw_list);
    player1.draw(draw_list);

    ImGui::EndChild();

}

void ClientUI::drawConfig() {
    bool changed = false;

    const char* title = getName().c_str();

    ImGui::BeginChild(title, ImVec2(0, 100), true); // hauteur FIXE
    ImGui::Text("%s", title);
    ImGui::Separator();

    int packetLoss = getPacketLoss();
    int ping = getPing();

    ImGui::SliderInt("Packet loss", &packetLoss, 0, 100);
    ImGui::InputInt("Ping", &ping);

    setPacketLoss(packetLoss);
    setPing(ping);

    ImGui::EndChild();
}