//
// Created by julie on 02/03/2026.
//

#include "../game/Player.hpp"

int main_test_ferolind()
{
    sf::RenderWindow window(sf::VideoMode({1600,900}), "Window-test_Ferolind");
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

    ImGui::SFML::Shutdown();
    return 0;
}