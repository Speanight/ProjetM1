//
// Created by julie on 04/03/2026.
//

// TODO : [MAP THING TO DO] add an element that can keep the weapon in a position, but it will depend on what is the map, it will be the same way as keeping the wall, (the traps ?) and the player after it ( @ ~ @) yeeee coool work in perspective

#include "Weapon.hpp"

// ===== CONSTRUCTORS =====
Weapon::Weapon(int id){
    applyID(id);
}

Weapon::Weapon(const Weapon& other)
    : w_id(other.w_id),
    w_height(other.w_height),
    w_width(other.w_width),
    w_type(other.w_type),
    w_atk_speed(other.w_atk_speed),
    w_rld(other.w_rld),
    w_range(other.w_range),
    w_transform(other.w_transform) {

}

// ===== GETTERS =====

int Weapon::getId() const {
    return w_id;
}

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

float Weapon::getTransform() const {
    return w_transform;
}

// ===== INTERNAL FUNCTION =====
void Weapon::applyID(short id) {
    w_id = id;
    switch (id) {
        case 0:
            w_height = 20.f;
            w_width = 8.f;
            w_type = Weapons::ARC;
            w_atk_speed = 0.2f;
            w_rld = 0.4f;
            w_range = 12.f;
            w_transform = 0.3f;
            break;
//        case 1: // weapon 1
//            w_id = 1;
//            w_height = 20.f;
//            w_width = 8.f;
//            w_type = Weapons::TRIANGLE;
//            w_atk_speed = 0.2f;
//            w_rld = 0.4f;
//            w_range = 12.f;
//            w_transform = 0.3f;
//            break;

        default: // Default weapon
            w_height = 16.f;
            w_width = 24.f;
            w_type = Weapons::TRIANGLE;
            w_atk_speed = 70.f;
            w_rld = 200.f;
            w_range = 15.0f;
            w_transform = 0.2f;
        break;
    }
}

