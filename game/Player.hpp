#ifndef PROJETM1_PLAYER_HPP
#define PROJETM1_PLAYER_HPP

#include "../communication/State.hpp"
#include "../Utils.hpp"
#include <string>

class Player {
private:
    // ====== SERVER ======
    unsigned short port;        // NEVER MOOVE THIS [use to create the client on the server and must be here
    unsigned short status = Status::WAITING_FOR_INIT;
    unsigned int clockSync;

    // ====== BASIC ======
    std::string name;
    sf::Color color;
    Position position = Position();

    // ====== WEAPON ======
    float radius = 0;               // must be saved as radiant so degree * ~1.111111 = radiant
    std::vector<short> weapons = {1, Weapons::SHIELD};
    short weapon = 0; // Index of weapons, points to which weapon is being used right now.
    Weapon wpn;

    // ====== ATTACK ======
    bool isAttacking;           // indicate if the player is attacking or not
    int timer_atk = -1;         // timer that indicate where we are in the animation, -1 stand for no animation
    int point;

public:
    Player(std::string name = "", sf::Color color = sf::Color(0,0,0,255), unsigned short port = 0, std::vector<short> weapons = {Weapons::SHIELD, 1});

    // Getters
    unsigned short getPort() const;
    unsigned short getStatus() const;
    unsigned int getClockSync() const;
    std::string getName() const;
    sf::Color getColor() const;
    Position getPosition() const;
    float getRadius() const;
    std::vector<short> getWeapons() const;
    Weapon getWpn() const;
    bool getIsAttacking() const;
    int getTimer_atk() const;
    int getPoint() const;

    // Setters
    void setPort(unsigned short port);
    void setStatus(unsigned short status);
    void setClockSync(unsigned int clockSync);
    void setName(std::string name);
    void setColor(sf::Color color);
    void setPosition(Position position);
    void setRadius(float radius);
    void setWeapons(std::vector<short> weapons);
    void setWpn(short weapon);
    void setIsAttacking(bool isAttacking);
    void setTimer_atk(int timer_atk);
    void setPoint(int point);

    // Functions
    void switchWeapon();
    void handleTimer_atk(int lastUpdate, int before);
};


#endif //PROJETM1_PLAYER_HPP
