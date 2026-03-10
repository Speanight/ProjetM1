#ifndef PROJETM1_INPUT_HPP
#define PROJETM1_INPUT_HPP

#include <SFML/Network/Packet.hpp>
#include "../Utils.hpp"

class Input {
private:
    float movementX;
    float movementY;
    float rotate;
    bool mode;                  // Signal to change the mode [false = no change, true = change]
    bool mode_enable;           // Permit to know if we can change the mode or no [false = no, true = yes]
    bool attack;                // Signal to inform the attack
    bool attack_enable;         // Permit to know if we can attack again or not
public:
    // Constructors
    Input();
    Input(float x, float y, bool attack);
    Input(float x, float y, float r, bool attack);
    Input(float x, float y, float r, bool mode, bool attack);

    // Getters
    float getMovementX();
    float getMovementY();
    float getRotate();
    bool getMode();
    bool getModeEnable();
    bool getAttack();
    bool getAttackEnable();

    // Setters
    void setMovementX(float x);
    void setMovementY(float y);
    void setRotate(float r);
    void setMode(bool mode);
    void setModeEnable(bool mode_enable);
    void setAttack(bool attack);
    void setAttackEnable(bool mode_enable);

    // Functions
    void handleInput(int inputCode, float value);
};

sf::Packet& operator<<(sf::Packet& packet, Input inputs);
sf::Packet& operator>>(sf::Packet& packet, Input& inputs);



#endif //PROJETM1_INPUT_HPP
