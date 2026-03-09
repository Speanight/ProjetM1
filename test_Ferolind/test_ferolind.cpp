//
// Created by julie on 02/03/2026.
//
// PLAYER.hpp
//
// Created by julie on 03/03/2026.
//
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <cmath>
#include "../Utils.hpp"
#include "../game/Weapon.hpp"
#ifndef PLAYER_HPP

//TODO : Put this values in the utils.hpp
class Player {private:
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

    // ===== DEFENSE =====
    bool  p_isTransforming = false;
    bool  p_isShield = false;          // état final
    float p_transformTimer = 0.f;
    float p_transformProgress = 0.f;   // 0 → 1

    // ===== WEAPON =====
    ImVec2 p_attackOffset = {0.f, 0.f};         // Actual offset of the attack
    ImVec2 p_attackDirection = {0.f, -1.f};     // Direction the weapon is aiming at
    Weapon p_weapon;                                // Weapon the player is having (datas)
    ImVec2 p_wpn_pos;                               // Weapon position based
    float p_wpn_radius;                             // Weapon orientation
public:
    // ===== CONSTRUCTORS =====
    Player(
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
    Player(const Player& other);

    //=================== SETTERS ===================
    void setWeapon(Weapon wpn);

    //=================== MOOVMENTS ===================
    void moovePlayer(ImVec2 direction, float deltaTime);
    void mooveWeapon(float angle, float deltaTime);

    //=================== ATTAQUE ===================
    void atkAction(Player& other);
    void atkAnimation(float deltaTime);

    //=================== DEFENSE ===================
    void defAction();
    void defAnimation(float deltaTime);

    //=================== COLLISIONS ===================
    void clampToMap(ImVec2 topRight, ImVec2 bottomLeft);
    void resolveCollision(Player& other);

    //=================== PRINT ===================
    void draw(ImDrawList* draw_list);


};
#define PLAYER_HPP

#endif //PLAYER_HPP

//# WEAPON ------------------------------------------------------------------------------------------
//
// Created by julie on 03/03/2026.
//
#include <math.h>


//PLAYER
// ===== CONSTRUCTORS =====
Player::Player(
        ImU32  p_color,
        ImVec2 p_position,
        float  p_radius,
        float  p_speed,

        int    p_point,

        bool  p_isAttacking,
        float p_attackTimer,

        ImVec2 p_attackOffset,
        ImVec2 p_attackDirection,
        Weapon p_weapon,
        ImVec2 p_wpn_pos,
        float p_wpn_radius,

        bool p_wpn_mode
        ) {
    this->p_color       = p_color;
    this->p_position    = p_position;
    this->p_radius      = p_radius;
    this->p_speed       = p_speed;

    this->p_point       = p_point;

    this->p_isAttacking = p_isAttacking;
    this->p_attackTimer = p_attackTimer;

    this->p_attackOffset    = p_attackOffset;
    this->p_attackDirection = p_attackDirection;
    this->p_weapon          = p_weapon;
    this->p_wpn_pos         = p_wpn_pos;
    this->p_wpn_radius      = p_wpn_radius;
}

Player::Player(const Player& other) :
    p_color(other.p_color),
    p_position(other.p_position),
    p_radius(other.p_radius),
    p_speed(other.p_speed),
    p_point(other.p_point),
    p_isAttacking(other.p_isAttacking),
    p_attackTimer(other.p_attackTimer),
    p_attackOffset(other.p_attackOffset),
    p_attackDirection(other.p_attackDirection),
    p_weapon(other.p_weapon),
    p_wpn_pos(other.p_wpn_pos),
    p_wpn_radius(other.p_wpn_radius)
{}

//=================== SETTERS ===================
void Player::setWeapon(const Weapon wpn) {
    this->p_weapon = wpn;
}

//=================== MOOVMENTS ===================

void Player::moovePlayer(ImVec2 direction, float deltaTime){
    p_position.x += direction.x * p_speed  * deltaTime;
    p_position.y += direction.y * p_speed  * deltaTime;
}

void Player::mooveWeapon(float angle, float deltaTime) {
    if(!p_isAttacking) {        // Cannot mooving the weapon while attacking
        p_wpn_radius += (angle*1.111111) * p_speed * deltaTime; //Conversion in gradiant
        if(p_wpn_radius > 360*1.111111 || p_wpn_radius < -360*1.111111) {
            p_wpn_radius = 0;
        }
    }
}

//=================== ATTAQUE ===================
void Player::atkAction(Player& other) {
    if (p_isAttacking || p_isShield || p_isTransforming)
        return;

    // Start the action
    p_isAttacking = true;
    p_attackTimer = 0.f;    //reloading the previous attack

    // TODO : check if the position of the enemy is good to be kicked
    ImVec2 diff = {
        other.p_position.x - p_position.x,
        other.p_position.y - p_position.y
    };

    float distance = sqrtf(diff.x*diff.x + diff.y*diff.y);

    if(distance <= p_radius * 2 + p_radius * p_weapon.getHeight() + p_weapon.getRange()){
        p_point += 1;
    }
}

void Player::atkAnimation(float deltaTime){
    if (!p_isAttacking)
        return;

    // printf("ATTACK !\n");
    p_attackTimer += deltaTime;

    float totalTime = p_weapon.getAttackSpeed() + p_weapon.getReload();

    if (p_attackTimer >= totalTime) {
        p_isAttacking = false;
        p_attackOffset = {0.f, 0.f};
        return;
    }

    float progress;
    if (p_attackTimer <= p_weapon.getAttackSpeed()) {
        progress = p_attackTimer / p_weapon.getAttackSpeed(); // 0 → 1
    }
    else {
        float backTime = p_attackTimer - p_weapon.getAttackSpeed();
        progress = 1.f - (backTime / p_weapon.getReload()); // 1 → 0
    }

    float attackDistance = p_weapon.getRange();

    p_attackOffset.x = attackDistance * progress;
    p_attackOffset.y = attackDistance * progress;
}

//=================== DEFENSE ===================
void Player::defAction()
{
    if (p_isTransforming)
        return;

    p_isTransforming = true;
    p_transformTimer = 0.f;
}

void Player::defAnimation(float deltaTime)
{
    if (!p_isTransforming)
        return;

    p_transformTimer += deltaTime;

    float duration = p_weapon.getTransform(); // w_transform

    float progress = p_transformTimer / duration;

    if (progress >= 1.f)
    {
        progress = 1.f;
        p_isTransforming = false;
        p_isShield = !p_isShield; // toggle état final
    }

    // Si on est déjà en shield → animation inverse
    if (p_isShield)
        p_transformProgress = 1.f - progress;
    else
        p_transformProgress = progress;
}

//=================== COLLISIONS ===================

void Player::clampToMap(ImVec2 topRight, ImVec2 bottomLeft){
    if (p_position.x - p_radius < topRight.x)
        p_position.x = topRight.x + p_radius;

    if (p_position.x + p_radius > bottomLeft.x)
        p_position.x = bottomLeft.x - p_radius;

    if (p_position.y - p_radius < topRight.y)
        p_position.y = topRight.y + p_radius;

    if (p_position.y + p_radius > bottomLeft.y)
        p_position.y = bottomLeft.y - p_radius;
}

void Player::resolveCollision(Player& other){
    ImVec2 diff = { other.p_position.x - p_position.x,
                    other.p_position.y - p_position.y };

    float distance = sqrtf(diff.x*diff.x + diff.y*diff.y);
    float minDistance = p_radius + other.p_radius;

    if (distance < minDistance)
    {
        if (distance == 0.f)
        {
            diff = {1.f, 0.f};
            distance = 1.f;
        }

        ImVec2 normal = { diff.x / distance, diff.y / distance };
        float penetration = minDistance - distance;

        other.p_position.x += normal.x * penetration;
        other.p_position.y += normal.y * penetration;
    }
}

void Player::draw(ImDrawList* draw_list){
    // Circle
    draw_list->AddCircleFilled(p_position, p_radius, p_color);

    // Triangle
    float triangleHeight = p_radius * 0.8f;
    float triangleWidth  = p_radius * 1.2f;

    // Angle en radians
    float angle = p_wpn_radius; //must be in radiant

    ImVec2 dir = {
        -sinf(angle),
        -cosf(angle)
    };

    // We can use the trigonometry to define the position of the top point of the wpn
    ImVec2 top = {
        p_position.x + dir.x * (p_radius + triangleHeight+2.f + p_attackOffset.x),
        p_position.y + dir.y * (p_radius + triangleHeight+2.f + p_attackOffset.y)
    };


    // then we can define the relative position of the left angle and the rigth angle, but first we need to find the center of the bottom of the triangle
    // for that, we can use
    ImVec2 bottom = {
        p_position.x + dir.x * (p_radius+2.f + p_attackOffset.x),
        p_position.y + dir.y * (p_radius+2.f + p_attackOffset.y)
    };


    ImVec2 perp = {
        -dir.y,
         dir.x
    };

    //And finally, we can calculate the point to the left and the rigth using this formula
    ImVec2 left = {
        bottom.x + perp.x * (triangleWidth / 2.f),
        bottom.y + perp.y * (triangleWidth / 2.f)
    };

    ImVec2 right = {
        bottom.x - perp.x * (triangleWidth / 2.f),
        bottom.y - perp.y * (triangleWidth / 2.f)
    };

    if (p_transformProgress < 1.f){
        // TRIANGLE (wpn)
        float scale = 1.f - p_transformProgress;

        ImVec2 scaledTop = {
            bottom.x + (top.x - bottom.x) * scale,
            bottom.y + (top.y - bottom.y) * scale
        };

        draw_list->AddTriangleFilled(scaledTop, left, right, p_color);
    }
    else
    {
        // ARC (bocle)
        float arcRadius = p_radius + 1.6f;

        float correctedAngle = p_wpn_radius - 3.14f * 0.5f;

        float angleStart = correctedAngle - 0.8f;
        float angleEnd   = correctedAngle + 0.8f;

        draw_list->PathArcTo(
            p_position,
            arcRadius,
            angleStart,
            angleEnd,
            20
        );

        draw_list->PathStroke(p_color, false, 4.f);
    }

    // Point
    std::string score = std::to_string(p_point);

    ImVec2 textSize = ImGui::CalcTextSize(score.c_str());

    ImVec2 textPos = {
        p_position.x - textSize.x / 2.f,
        p_position.y - textSize.y / 2.f
    };

    draw_list->AddText(ImGui::GetFont(),20.f,textPos, IM_COL32(0,0,0,255), score.c_str());
}

//#-----------------------------------------------------

