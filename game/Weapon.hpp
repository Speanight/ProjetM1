//
// Created by julie on 04/03/2026.
//
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <cmath>
#include "../Utils.hpp"
#ifndef WEAPON_HPP
#define WEAPON_HPP

class Weapon {
    private :
    // ===== FORM =====
    float w_height;         // wpn size
    float w_width;          // wpn size
    int w_type;             // TODO : [IDEA] different type of weapon that have a different behavior and look (like square or circle that can be throwable ?)

    // ===== BEHAVIOR =====
    float w_atk_speed;      // wpn attack speed (will depend of the game clock)
    float w_rld;            // wpn realod speed (will depend of the game clock)
    float w_range;          // wpn range
    float w_transform;      // wpn transformation time

public:
    // ===== CONSTRUCTORS =====
    Weapon(float height = 0.8f, float width = 1.2f, float type = 0.f, float atk_spd = 0.1f, float rld=0.3f, float range = 10.f, float transform = 0.2f);
    Weapon(const Weapon& other);

    // ===== GETTERS =====
    float  getHeight() const;
    float  getWidth() const;
    int    getType() const;

    float  getAttackSpeed() const;
    float  getReload() const;
    float  getRange() const;

    float getTransform() const;

    // ===== SETTERS =====
    void setHeight(float height);
    void setWidth(float width);
    void setType(int type);

    void setAttackSpeed(float atk_spd);
    void setReload(float rld);
    void setRange(float range);

};
#endif //WEAPON_HPP
