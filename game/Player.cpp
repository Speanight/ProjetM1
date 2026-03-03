//
// Created by julie on 03/03/2026.
//
#include "Player.hpp"

// ================= CREATION =================

Player::Player(float radius, sf::Vector2f centerPosition, sf::Color color, float speed) : m_radius(radius), m_speed(speed) {
    // ================= BODY =================
    m_body.setRadius(m_radius);
    m_body.setFillColor(color);
    m_body.setOrigin({m_radius, m_radius});
    m_body.setPosition(centerPosition);

    // ================= WEAPON =================
    float triangleHeight = m_radius * 0.8f;
    float triangleWidth  = m_radius * 1.2f;

    m_weapon.setPointCount(3);
    m_weapon.setPoint(0, {0.f, -m_radius - triangleHeight});      // pointe
    m_weapon.setPoint(1, {-triangleWidth/2.f, -m_radius});
    m_weapon.setPoint(2, { triangleWidth/2.f, -m_radius});

    m_weapon.setFillColor(color);
    m_weapon.setPosition(centerPosition);
}

// ================= GETTERS =================

sf::Vector2f Player::getPosition() const {
    return m_body.getPosition();
}

float Player::getRadius() const {
    return m_radius;
}

// ================= MOVEMENT =================
void Player::move(sf::Vector2f direction, float deltaTime){
    sf::Vector2f velocity = direction * m_speed * deltaTime;

    m_body.move(velocity);
    m_weapon.move(velocity);
}

// ================= SCREEN COLLISION =================
void Player::clampToWindow(const sf::RenderWindow& window){
    sf::Vector2u winSize = window.getSize();
    sf::Vector2f pos = m_body.getPosition();

    if (pos.x - m_radius < 0.f)
        pos.x = m_radius;

    if (pos.x + m_radius > winSize.x)
        pos.x = winSize.x - m_radius;

    if (pos.y - m_radius < 0.f)
        pos.y = m_radius;

    if (pos.y + m_radius > winSize.y)
        pos.y = winSize.y - m_radius;

    m_body.setPosition(pos);
    m_weapon.setPosition(pos);
}

void Player::resolveCollision(Player& other)
{
    sf::Vector2f diff = other.getPosition() - getPosition();
    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    float minDistance = m_radius + other.getRadius();

    if (distance < minDistance)
    {
        if (distance == 0.f)
        {
            diff = {1.f, 0.f};
            distance = 1.f;
        }

        sf::Vector2f normal = diff / distance;
        float penetration = minDistance - distance;

        other.m_body.move(normal * penetration);
        other.m_weapon.move(normal * penetration);
    }
}