//
// Created by julie on 03/03/2026.
//
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <cmath>

#ifndef PLAYER_HPP

class Player {
private:
    ImVec2 m_center;
    float  m_radius;
    float  m_speed;
    ImU32  m_color;
    int    m_point;

public:
    //TODO : weapon direction to transmit in packet
    Player(float radius, ImVec2 center, ImU32 color, float speed = 400.f, int point=0) :
        m_radius(radius),
        m_center(center),
        m_color(color),
        m_speed(speed),
        m_point(point)
    {}

    //=================== GETTERS ===================
    ImVec2 getPosition() const;
    float  getRadius() const;

    //=================== MOOVMENTS ===================
    void move(ImVec2 direction, float deltaTime);
    void atk();

    //=================== COLLISIONS ===================
    void clampToChild(ImVec2 childMin, ImVec2 childMax);
    void resolveCollision(Player& other);

    //=================== PRINT ===================
    void draw(ImDrawList* draw_list);
};

#define PLAYER_HPP

#endif //PLAYER_HPP
