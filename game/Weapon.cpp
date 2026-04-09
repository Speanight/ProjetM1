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
    w_damage(other.w_damage),
    w_transform(other.w_transform) {

}

// ===== GETTERS =====
short Weapon::getId() const {
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

int Weapon::getDamage() const {
    return w_damage;
}

float Weapon::getTransform() const {
    return w_transform;
}

// ===== INTERNAL FUNCTION =====
/**
 * adapt the weapon depending on the id we give to it, that way we only have to know the Weapon ID to get all the datas about it (make the weapon changement and selection easier)
 * [0] shield
 * [1-3] knives (triangle weapon)
 * [4-5] sticks (rectagle weapon)
 * [6-8] rocks (circle weapon)
 * [10-12] special weapons (any)
 * default => return to weapon 1 (triangle)
 * @param id    : int that goes from 0 to 12 to inform the weapon the user have
 */
void Weapon::applyID(short id) {
    w_id = id;
    switch (id) {
        case Weapons::SHIELD: {
            w_height = 20.f;
            w_width = 8.f;
            w_type = Weapons::SHIELD;
            w_atk_speed = 0.2f;
            w_rld = 0.4f;
            w_range = 12.f;
            w_transform = 0.3f;
            break;
        }
/*------------------------------------------*/
        case 1: {
            w_id = 1;
            w_height = 15.f;
            w_width = 22.f;
            w_type = Weapons::TRIANGLE;
            w_atk_speed = 70.f;
            w_rld = 200.f;
            w_range = 15.f;
            w_damage = 10;
            w_transform = 5.f;
            break;
        }
        case 2: {
            w_id = 2;
            w_height = 20.f;
            w_width = 10.f;
            w_type = Weapons::TRIANGLE;
            w_atk_speed = 100.f;
            w_rld = 200.f;
            w_range = 20.f;
            w_damage = 15;
            w_transform = 5.f;
            break;
        }
        case 3: {
            w_id = 3;
            w_height = 20.f;
            w_width = 16.f;
            w_type = Weapons::TRIANGLE;
            w_atk_speed = 80.f;
            w_rld = 50.f;
            w_range = 60.f;
            w_damage = 3;
            w_transform = 5.f;
            break;
        }
        case 4: {
            w_id = 4;
            w_height = 30.f;
            w_width = 10.f;
            w_type = Weapons::RECTANGLE;
            w_atk_speed = 70.f;
            w_rld = 100.f;
            w_range = std::numbers::pi/2;
            w_damage = 10;
            w_transform = 5.f;
            break;
        }
        case 5: {
            w_id = 5;
            w_height = 20.f;
            w_width = 10.f;
            w_type = Weapons::RECTANGLE;
            w_atk_speed = 50.f;
            w_rld = 300.f;
            w_range = std::numbers::pi/2;
            w_damage = 15;
            w_transform = 5.f;
            break;
        }
        case 6: {
            w_id = 6;
            w_height = 10.f;
            w_width = 10.f;
            w_type = Weapons::CIRCLE;
            w_atk_speed = 300.f;
            w_rld = 500.f;
            w_range = 500;
            w_damage = 15;
            w_transform = 5.f;
            break;
        }
        case 7: {
            w_id = 7;
            w_height = 20.f;
            w_width = 20.f;
            w_type = Weapons::CIRCLE;
            w_atk_speed = 300.f;
            w_rld = 700.f;
            w_range = 500;
            w_damage = 20;
            w_transform = 5.f;
            break;
        }
        case 8: {
            w_id = 8;
            w_height = 30.f;
            w_width = 30.f;
            w_type = Weapons::CIRCLE;
            w_atk_speed = 300.f;
            w_rld = 1300.f;
            w_range = 500;
            w_damage = 30;
            w_transform = 5.f;
            break;
        }
/*----------------------------------------*/
        case 10: {      // SUPER WEAPON FOR TWIG
            w_id = 10;
            w_height = 40.f;
            w_width = 50.f;
            w_type = Weapons::TRIANGLE;
            w_atk_speed = 30.f;
            w_rld = 50.f;
            w_range = 40.f;
            w_damage = 1;
            w_transform = 5.f;
            break;
        }
        case 11: {      // SUPER WEAPON FOR PONY
            w_id = 11;
            w_height = 5.f;
            w_width = 10.f;
            w_type = Weapons::CIRCLE;
            w_atk_speed = 500.f;
            w_rld = 1000.f;
            w_range = 500.f;
            w_damage = 1000;
            w_transform = 5.f;
            break;
        }
        case 12: {      // SUPER WEAPON FOR BESCHER
            w_id = 12;
            w_height = 50.f;
            w_width = 5.f;
            w_type = Weapons::RECTANGLE;
            w_atk_speed = 50.f;
            w_rld = 50.f;
            w_range = std::numbers::pi;
            w_damage = 1000;
            w_transform = 5.f;
            break;
        }
        default: {
            // Default weapon
            w_id = 1;
            w_height = 16.f;
            w_width = 24.f;
            w_type = Weapons::TRIANGLE;
            w_atk_speed = 70.f;
            w_rld = 200.f;
            w_range = 15.f;
            w_damage = 10;
            w_transform = 5.f;
            break;
        }
    }
}

