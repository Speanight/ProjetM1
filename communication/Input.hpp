#ifndef PROJETM1_INPUT_HPP
#define PROJETM1_INPUT_HPP

#include <SFML/Network/Packet.hpp>
#include "../Utils.hpp"
#include <cmath>
#include <iostream>

class Input {
private:
    // TODO: unsigned int limit is at 4,294,967,295. This means it should be reset to 0 on round start!
    unsigned int id;
    short movementX;
    short movementY;
    float rotate;
    bool onController;
    bool changeWpn;             // Signal to change weapon
    bool mode_enable;           // Permit to know if we can change the changeWpn or no [false = no, true = yes]
    bool attack;                // Signal to inform the attack
    bool attack_enable;         // Permit to know if we can attack again or not
public:
    // Constructors
    Input(unsigned int id=0, float x=0, float y=0, float r=0, bool mode=false, bool attack=false, bool onController = false);

    // Getters
    unsigned int getId() const;
    float getMovementX() const;
    float getMovementY() const;
    float getRotate() const;
    bool getChangeWpn() const;
    bool getModeEnable() const;
    bool getAttack() const;
    bool getAttackEnable() const;
    bool getOnController() const;

    // Setters
    void setId(unsigned int id);
    void setMovementX(float x);
    void setMovementY(float y);
    void setRotate(float r);
    void setChangeWpn(bool changeWpn);
    void setModeEnable(bool mode_enable);
    void setAttack(bool attack);
    void setAttackEnable(bool mode_enable);
    void setOnController(bool onController);

    // Functions
    void handleInput(int inputCode, float value);

    bool operator==(const Input& other) const;
};

sf::Packet& operator<<(sf::Packet& packet, Input inputs);
sf::Packet& operator>>(sf::Packet& packet, Input& inputs);

std::ostream &operator<<(std::ostream& os, const Input& inputs);

#endif //PROJETM1_INPUT_HPP
