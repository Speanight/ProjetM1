//
// Created by julie on 04/03/2026.
//

// TODO : [MAP THING TO DO] add an element that can keep the weapon in a position, but it will depend on what is the map, it will be the same way as keeping the wall, (the traps ?) and the player after it ( @ ~ @) yeeee coool work in perspective

#include "Weapon.hpp"

// ===== CONSTRUCTORS =====


// Weapon::Weapon( float height, float width, float type, float atk_spd, float rld, float range, float transform ){
//     // ===== FORM =====
//     w_height    = height;
//     w_width     = width;
//     w_type      = type;             // TODO : [IDEA] different type of weapon that have a different behavior and look (like square or circle that can be throwable ?)
//
//     // ===== BEHAVIOR =====
//     w_atk_speed = atk_spd;
//     w_rld       = rld;
//     w_range     = range;
//     w_transform = transform;
// }

Weapon::Weapon(const int id) {
    switch (id) {
        case 1:
            printf("WPN DOES NOT EXIST");
            this->w_id = 1;
            break;
        default :               // Default wpn set
            this->w_id = 0;
            this->w_height = 16.f;
            this->w_width = 24.f;
            this->w_type = 0;
            this->w_atk_speed = 0.1f;
            this->w_rld = 0.3f;
            this->w_range = 10.0f;
            this->w_transform = 0.2f;
            break;
    }
}

Weapon::Weapon(int id, float height = 16.f, float width = 24.f, float type = 0.f, float atk_spd = 0.1f, float rld=0.3f, float range = 10.f, float transform = 0.2f);

Weapon::Weapon(const Weapon& other)
    :w_id(other.w_id),
    w_height(other.w_height),
    w_width(other.w_width),
    w_type(other.w_type),
    w_atk_speed(other.w_atk_speed),
    w_rld(other.w_rld),
    w_range(other.w_range),
    w_transform(other.w_transform)
{}

// ===== GETTERS =====

float Weapon::getHeight() const {
    return w_height;
}

float Weapon::getWidth() const {
    return w_width;
}

int Weapon::getType() const {
    return w_type;
}

float Weapon::getAttackSpeed() const {
    return w_atk_speed;
}

float Weapon::getReload() const {
    return w_rld;
}

float Weapon::getRange() const {
    return w_range;
}

float Weapon::getTransform() const {return w_transform;}

// ===== SETTERS =====

void Weapon::setHeight(float height) {
    w_height = height;
}

void Weapon::setWidth(float width) {
    w_width = width;
}

void Weapon::setType(int type) {
    w_type = type;
}

void Weapon::setAttackSpeed(float atk_spd) {
    w_atk_speed = atk_spd;
}

void Weapon::setReload(float rld) {
    w_rld = rld;
}

void Weapon::setRange(float range) {
    w_range = range;
}

