#ifndef PROJETM1_INPUT_HPP
#define PROJETM1_INPUT_HPP

#include <SFML/Network/Packet.hpp>
#include "../Utils.hpp"

class Input {
private:
    // TODO: unsigned int limit is at 4,294,967,295. This means it should be reset to 0 on round start!
    unsigned int id;
    float movementX;
    float movementY;
    float rotate;
    bool mode;                  // Signal to change the mode [false = no change, true = change]
    bool mode_enable;           // Permit to know if we can change the mode or no [false = no, true = yes]
    bool attack;                // Signal to inform the attack
    bool attack_enable;         // Permit to know if we can attack again or not
    int wpn_id;
public:
    // Constructors
    Input(unsigned int id=0, float x=0, float y=0, float r=0, bool mode=false, bool attack=false, int wpn_id=0);

    // Getters
    unsigned int getId();
    float getMovementX();
    float getMovementY();
    float getRotate();
    bool getMode();
    bool getModeEnable();
    bool getAttack();
    bool getAttackEnable();
    int getWpnID();

    // Setters
    void setId(unsigned int id);
    void setMovementX(float x);
    void setMovementY(float y);
    void setRotate(float r);
    void setMode(bool mode);
    void setModeEnable(bool mode_enable);
    void setAttack(bool attack);
    void setAttackEnable(bool mode_enable);
    void setWpnID(int wpn_id);

    // Functions
    void handleInput(int inputCode, float value);
};

sf::Packet& operator<<(sf::Packet& packet, Input inputs);
sf::Packet& operator>>(sf::Packet& packet, Input& inputs);



#endif //PROJETM1_INPUT_HPP
