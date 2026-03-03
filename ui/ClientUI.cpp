#include "ClientUI.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/Color.hpp>

ClientUI::ClientUI(const sf::Clock clock, std::string name, sf::Color color) : Client(clock, name, color) {}

void ClientUI::drawGame() { // Game space
        const char* title = getName().c_str();

        ImGui::BeginChild(title, ImVec2(0, 400), true);

        ImVec2 childMin = ImGui::GetWindowPos();
        ImVec2 childMax = {
            childMin.x + ImGui::GetWindowSize().x,
            childMin.y + ImGui::GetWindowSize().y
        };

        static sf::Clock deltaClock;
        // TODO: vVv This function causes player 2 to moves slower.
        float deltaTime = deltaClock.restart().asSeconds();

        // ========= INPUT =========
        ImVec2 dir = {0.f, 0.f};

        Input inputs = getInputs();

        dir.x += 1.f * inputs.getMovementX();
        dir.y += 1.f * inputs.getMovementY();

        // ========= UPDATE =========
        move(dir, deltaTime);
        clampToChild(childMin, childMax);

        for (auto & [name, other] : opponents) {
            other.clampToChild(childMin, childMax);
            resolveCollision(other);
        }

        // ========= DRAW =========
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        drawPlayer(draw_list);

        for (auto & [name, other] : opponents) {
            other.drawPlayer(draw_list);
        }

        ImGui::EndChild();
}


void ClientUI::addOpponent(sf::Clock clock, const std::string& name, sf::Color color) {
    opponents.insert(std::make_pair(name, ClientUI(clock, name, color)));
}

void ClientUI::drawPlayer(ImDrawList* draw_list) {
    // Cercle
    draw_list->AddCircleFilled(ImVec2(getPosition().getX(), getPosition().getY()), m_radius, IM_COL32(int{getColor().r}, int{getColor().g}, int{getColor().b}, int{getColor().a}));

    // Triangle
    float triangleHeight = m_radius * 0.8f;
    float triangleWidth  = m_radius * 1.2f;

    ImVec2 top   = { getPosition().getX(),
                     getPosition().getY() - m_radius - triangleHeight };

    ImVec2 left  = { getPosition().getX() - triangleWidth/2.f,
                     getPosition().getY() - m_radius };

    ImVec2 right = { getPosition().getX() + triangleWidth/2.f,
                     getPosition().getY() - m_radius };

    draw_list->AddTriangleFilled(top, left, right, IM_COL32(int{getColor().r}, int{getColor().g}, int{getColor().b}, int{getColor().a}));
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