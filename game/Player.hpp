//
// Created by julie on 03/03/2026.
//
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <cmath>

#ifndef PLAYER_HPP

/*
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
*/

class Player
{
private:
    ImVec2 m_center;
    float  m_radius;
    float  m_speed;
    ImU32  m_color;

public:
    Player(float radius, ImVec2 center, ImU32 color, float speed = 400.f)       //TODO : weapon direction to transmit in packet
        : m_radius(radius), m_center(center), m_color(color), m_speed(speed)
    {}

    ImVec2 getPosition() const { return m_center; }
    float  getRadius() const { return m_radius; }

    void move(ImVec2 direction, float deltaTime)
    {
        m_center.x += direction.x * m_speed * deltaTime;
        m_center.y += direction.y * m_speed * deltaTime;
    }

    void clampToChild(ImVec2 childMin, ImVec2 childMax)
    {
        if (m_center.x - m_radius < childMin.x)
            m_center.x = childMin.x + m_radius;

        if (m_center.x + m_radius > childMax.x)
            m_center.x = childMax.x - m_radius;

        if (m_center.y - m_radius < childMin.y)
            m_center.y = childMin.y + m_radius;

        if (m_center.y + m_radius > childMax.y)
            m_center.y = childMax.y - m_radius;
    }

    void resolveCollision(Player& other)
    {
        ImVec2 diff = { other.m_center.x - m_center.x,
                        other.m_center.y - m_center.y };

        float distance = sqrtf(diff.x*diff.x + diff.y*diff.y);
        float minDistance = m_radius + other.m_radius;

        if (distance < minDistance)
        {
            if (distance == 0.f)
            {
                diff = {1.f, 0.f};
                distance = 1.f;
            }

            ImVec2 normal = { diff.x / distance, diff.y / distance };
            float penetration = minDistance - distance;

            other.m_center.x += normal.x * penetration;
            other.m_center.y += normal.y * penetration;
        }
    }

    void draw(ImDrawList* draw_list)
    {
        // Cercle
        draw_list->AddCircleFilled(m_center, m_radius, m_color);

        // Triangle
        float triangleHeight = m_radius * 0.8f;
        float triangleWidth  = m_radius * 1.2f;

        ImVec2 top   = { m_center.x,
                         m_center.y - m_radius - triangleHeight };

        ImVec2 left  = { m_center.x - triangleWidth/2.f,
                         m_center.y - m_radius };

        ImVec2 right = { m_center.x + triangleWidth/2.f,
                         m_center.y - m_radius };

        draw_list->AddTriangleFilled(top, left, right, m_color);
    }
};

#define PLAYER_HPP

#endif //PLAYER_HPP
