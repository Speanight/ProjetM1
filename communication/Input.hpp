#ifndef PROJETM1_INPUT_HPP
#define PROJETM1_INPUT_HPP

#include <SFML/Network/Packet.hpp>
#include "../Utils.hpp"
#include <cmath>

class Input {
private:
    // TODO: unsigned int limit is at 4,294,967,295. This means it should be reset to 0 on round start!
    unsigned int id;
    float movementX;
    float movementY;
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
    unsigned int getId();
    float getMovementX();
    float getMovementY();
    float getRotate();
    bool getChangeWpn();
    bool getModeEnable();
    bool getAttack();
    bool getAttackEnable();
    bool getOnController();

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
};

sf::Packet& operator<<(sf::Packet& packet, Input inputs);
sf::Packet& operator>>(sf::Packet& packet, Input& inputs);



#endif //PROJETM1_INPUT_HPP
