#include "ClientUI.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/Color.hpp>
#include "../game/Player.hpp"

ClientUI::ClientUI(const sf::Clock clock, std::string name) : Client(clock, name) {}

void ClientUI::drawGame() { // Game space
    const char* title = getName().c_str();

    sf::RenderWindow window(sf::VideoMode({400,400}), "Window-player");
    window.setFramerateLimit(240);

    ImGui::SFML::Init(window);
    sf::Clock deltaClock;

    Player player1(50.f, {200.f, 200.f}, sf::Color::Green);
    Player player2(50.f, {600.f, 400.f}, sf::Color::Red);

    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        sf::Time dt = deltaClock.restart();
        float deltaTime = dt.asSeconds();

        ImGui::SFML::Update(window, dt);

        // ================= INPUT PLAYER 1 =================
        sf::Vector2f direction1(0.f, 0.f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            direction1.y -= 1.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            direction1.y += 1.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            direction1.x -= 1.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            direction1.x += 1.f;

        // ================= UPDATE =================
        player1.move(direction1, deltaTime);

        player1.clampToWindow(window);
        player2.clampToWindow(window);

        player1.resolveCollision(player2);

        // ================= RENDER =================
        window.clear(sf::Color::White);

        window.draw(player1);
        window.draw(player2);

        ImGui::SFML::Render(window);
        window.display();
    }

    // ImGui::SFML::Shutdown();

    ImGui::Separator();

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