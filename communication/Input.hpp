#ifndef PROJETM1_INPUT_HPP
#define PROJETM1_INPUT_HPP

#include <SFML/Network/Packet.hpp>
#include "../Utils.hpp"

class Input {
private:
    float movementX;
    float movementY;
    float rotate;
    bool mode;                  // Signal to change the mode
    bool attack;
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
    bool getAttack();


    // Setters
    void setMovementX(float x);
    void setMovementY(float y);
    void setRotate(float r);
    void setMode(bool mode);
    void setAttack(bool attack);

    // Functions
    void handleInput(int inputCode, float value);
};

sf::Packet& operator<<(sf::Packet& packet, Input inputs);
sf::Packet& operator>>(sf::Packet& packet, Input& inputs);



#endif //PROJETM1_INPUT_HPP
