//
// Created by julie on 03/03/2026.
//
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <cmath>
#include "../Utils.hpp"
#ifndef PLAYER_HPP

//TODO : Put this values in the utils.hpp
/*
// ===== PLAYER =====
float M_SPEED = 0.400f;
float M_PLAYER_RADIUS = 20.f;
float M_TRIANGLEHEIGTH = M_PLAYER_RADIUS * 0.8f;
float M_TRIANGLEWIDTH  = M_PLAYER_RADIUS * 1.2f;

// ===== POINT =====
int M_ORIGIN_POINT = 0;
float M_TEXTSIZE = 20.f;

// ===== ATTACK =====
float M_TIME_ATK = 0.1f;
float M_TIME_RELOAD = 0.2f;
float M_RANGE = 0.10f;
*/

class Player {
private:
    // ===== PLAYER =====
    ImVec2 m_center;
    float  m_radius;
    float  m_speed;
    ImU32  m_color;

    // ===== POINT =====
    int    m_point;

    // ===== ATTACK =====
    bool  m_isAttacking = false;
    float m_attackTimer = 0.f;

    float m_attack = 0.1f;
    float m_reload = 0.2f;

    float m_range = 10.f;

    ImVec2 m_attackOffset = {0.f, 0.f};
    ImVec2 m_attackDirection = {0.f, -1.f};

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
    void atk(Player& other);
    void updateAttack(float deltaTime);

    //=================== COLLISIONS ===================
    void clampToChild(ImVec2 childMin, ImVec2 childMax);
    void resolveCollision(Player& other);

    //=================== PRINT ===================
    void draw(ImDrawList* draw_list);
};

#define PLAYER_HPP

#endif //PLAYER_HPP
