#ifndef PROJETM1_INPUT_HPP
#define PROJETM1_INPUT_HPP

#include <SFML/Network/Packet.hpp>
#include "../Utils.hpp"

class Input {
private:
    float movementX;
    float movementY;
    float rotateRigth;
    float rotateLeft;
    bool attack;
public:
    // Constructors
    Input();
    Input(float x, float y, bool attack);
    Input(float x, float y, float r, float l, bool attack)

    // Getters
    float getMovementX();
    float getMovementY();
    float getRotateRigth();
    float getRotateLeft();
    bool getAttack();

    // Setters
    void setMovementX(float x);
    void setMovementY(float y);
    void setRotateRigth(float r);
    void setRotateLeft(float l);
    void setAttack(bool attack);

    // Functions
    void handleInput(int inputCode, float value);
};

sf::Packet& operator<<(sf::Packet& packet, Input inputs);
sf::Packet& operator>>(sf::Packet& packet, Input& inputs);



#endif //PROJETM1_INPUT_HPP
