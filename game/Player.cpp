//
// Created by julie on 03/03/2026.
//
#include "Player.hpp"

//=================== GETTERS ===================

ImVec2 Player::getPosition() const {
    return m_center;
}

float  Player::getRadius() const {
    return m_radius;
}

//=================== MOOVMENTS ===================

void Player::move(ImVec2 direction, float deltaTime){
    m_center.x += direction.x * m_speed * deltaTime;
    m_center.y += direction.y * m_speed * deltaTime;
}

void Player::atk() {

}

//=================== COLLISIONS ===================

void Player::clampToChild(ImVec2 childMin, ImVec2 childMax){
    if (m_center.x - m_radius < childMin.x)
        m_center.x = childMin.x + m_radius;

    if (m_center.x + m_radius > childMax.x)
        m_center.x = childMax.x - m_radius;

    if (m_center.y - m_radius < childMin.y)
        m_center.y = childMin.y + m_radius;

    if (m_center.y + m_radius > childMax.y)
        m_center.y = childMax.y - m_radius;
}

void Player::resolveCollision(Player& other){
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

void Player::draw(ImDrawList* draw_list){
    // Circle
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

    // Point
    std::string score = std::to_string(m_point);

    ImVec2 textSize = ImGui::CalcTextSize(score.c_str());

    ImVec2 textPos = {
        m_center.x - textSize.x / 2.f,
        m_center.y - textSize.y / 2.f
    };

    draw_list->AddText(ImGui::GetFont(),20.f,textPos, IM_COL32(0,0,0,255), score.c_str());
}