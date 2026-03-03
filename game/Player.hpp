//
// Created by julie on 03/03/2026.
//
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <cmath>

#ifndef PLAYER_HPP

class Player : public sf::Drawable
{
private:
    sf::CircleShape m_body;
    sf::ConvexShape m_weapon;

    float m_radius;
    float m_speed;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(m_body, states);
        target.draw(m_weapon, states);
    }

public:
    Player(float radius, sf::Vector2f centerPosition, sf::Color color, float speed = 400.f);

    // ================= GETTERS =================
    sf::Vector2f getPosition() const;
    float getRadius() const;

    // ================= MOVEMENT =================
    void move(sf::Vector2f direction, float deltaTime);


    // ================= SCREEN COLLISION =================
    void clampToWindow(const sf::RenderWindow& window);

    // ================= PLAYER COLLISION (push other) =================
    void resolveCollision(Player& other);
};

#define PLAYER_HPP

#endif //PLAYER_HPP
