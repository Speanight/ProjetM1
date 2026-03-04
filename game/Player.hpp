//
// Created by julie on 03/03/2026.
//
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <cmath>
#include "../Utils.hpp"
#include "Weapon.hpp"
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
    ImVec2 m_center_wpn = {m_center.x + m_attackOffset.x, m_center.y + m_attackOffset.y};
    float m_radius_wpn;
    bool  m_isAttacking = false;
    float m_attackTimer = 0.f;

    float m_attack = 0.1f;
    float m_reload = 0.2f;

    float m_range = 10.f;

    ImVec2 m_attackOffset = {0.f, 0.f};
    ImVec2 m_attackDirection = {0.f, -1.f};

    // ===== SWITCH =====
    bool m_wpn_mode = true;

public:
    //TODO : weapon direction to transmit in packet
    Player(float radius, ImVec2 center, ImU32 color, float speed = 400.f, int point=0, float m_radius_wpn = 0.f) :
        m_radius(radius),
        m_center(center),
        m_color(color),
        m_speed(speed),
        m_point(point),
        m_radius_wpn(point)
    {}

    //=================== GETTERS ===================
    ImVec2 getPosition() const;

    //=================== MOOVMENTS ===================
    void move(ImVec2 direction, float deltaTime);
    void UpdateWpn(float angle, float deltaTime);

    //=================== ATTAQUE ===================
    void atk(Player& other);
    void updateAttack(float deltaTime);

    //=================== COLLISIONS ===================
    void clampToChild(ImVec2 childMin, ImVec2 childMax);
    void resolveCollision(Player& other);

    //=================== PRINT ===================
    void draw(ImDrawList* draw_list);
};

class Player2 {private:
    // ===== PLAYER =====
    ImU32  p_color;                                 // Color of the player
    ImVec2 p_position;                              // Position of the center of the player
    float  p_radius;                                // Radius of the circle representing the player
    float  p_speed;                                 // Player speed

    // ===== POINT =====
    int    p_point;                                 // Player point

    // ===== ATTACK =====
    bool  p_isAttacking;                            // State that inform if the player is attacking
    float p_attackTimer;                            // Timer for the attack

    ImVec2 p_attackOffset = {0.f, 0.f};         // Actual offset of the attack
    ImVec2 p_attackDirection = {0.f, -1.f};     // Direction the weapon is aiming at
    Weapon p_weapon;                                // Weapon the player is having (datas)
    ImVec2 p_wpn_pos;                               // Weapon position based
    float p_wpn_radius;                             // Weapon orientation

    // ===== SWITCH =====
    bool p_wpn_mode = true;                         // State that inform if the player is in defense or attacking mode (true : attack, false : defense)
public:
    // ===== CONSTRUCTORS =====
    Player2(
        ImU32  p_color = IM_COL32(255, 255, 255, 255),
        ImVec2 p_position = ImVec2(0, 0),
        float  p_radius = 20.f,
        float  p_speed = 400.f,

        int    p_point = 0,

        bool  p_isAttacking = false,
        float p_attackTimer = 0.f,

        ImVec2 p_attackOffset = {0.f, 0.f},
        ImVec2 p_attackDirection = {0.f, -1.f},
        Weapon p_weapon = Weapon(),
        ImVec2 p_wpn_pos = ImVec2(0, 0),
        float p_wpn_radius = 0.f,

        bool p_wpn_mode = true
        );
    Player2(const Player2& other);

    //=================== SETTERS ===================
    void setWeapon(Weapon wpn);

    //=================== MOOVMENTS ===================
    void moovePlayer(ImVec2 direction, float deltaTime);
    void mooveWeapon(float angle, float deltaTime);

    //=================== ATTAQUE ===================
    void atkAction(Player2& other);
    void atkAnimation(float deltaTime);

    //=================== COLLISIONS ===================
    void clampToMap(ImVec2 topRight, ImVec2 bottomLeft);
    void resolveCollision(Player2& other);

    //=================== PRINT ===================
    void draw(ImDrawList* draw_list);


};
#define PLAYER_HPP

#endif //PLAYER_HPP
