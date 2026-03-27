#include "Player.hpp"

Player::Player(std::string name, sf::Color color, unsigned short port, std::vector<short> weapons) {
    this->name = std::move(name);
    this->color = color;
    this->port = port;
    this->radius = std::numbers::pi/2;
    this->weapon = 0;
    this->weapons = weapons;
    this->timer_atk = -1;
    this->isAttacking = false;
    this->status = Status::WAITING_FOR_INIT;
    this->wpn = Weapon(weapons[weapon]);
}

// Getters
unsigned short Player::getPort() const {
    return this->port;
}

unsigned short Player::getStatus() const {
    return this->status;
}

unsigned int Player::getClockSync() const {
    return this->clockSync;
}

std::string Player::getName() const {
    return this->name;
}

sf::Color Player::getColor() const {
    return this->color;
}

Position Player::getPosition() const {
    return this->position;
}

float Player::getRadius() const {
    return this->radius;
}

std::vector<short> Player::getWeapons() const {
    return this->weapons;
}

Weapon Player::getWpn() const {
    return this->wpn;
}

bool Player::getIsAttacking() const {
    return this->isAttacking;
}

int Player::getTimer_atk() const {
    return this->timer_atk;
}

int Player::getPoint() const {
    return this->point;
}

// Setters
void Player::setPort(unsigned short port) {
    this->port = port;
}

void Player::setStatus(unsigned short status) {
    this->status = status;
}

void Player::setClockSync(unsigned int clockSync) {
    this->clockSync = clockSync;
}

void Player::setName(std::string name) {
    this->name = name;
}

void Player::setColor(sf::Color color) {
    this->color = color;
}

void Player::setPosition(Position position) {
    this->position.setX(position.getX());
    this->position.setY(position.getY());
}

void Player::setRadius(float radius) {
    this->radius = std::fmod(radius, 2.f * std::numbers::pi);
}

void Player::setWeapons(std::vector<short> weapons) {
    this->weapons = weapons;
}

void Player::setWpn(short weapon) {
    if (std::find(this->weapons.begin(), this->weapons.end(), weapon) != this->weapons.end()) {
        this->wpn.applyID(weapon);
    }
}

void Player::setIsAttacking(bool isAttacking) {
    this->isAttacking = isAttacking;
}

void Player::setTimer_atk(int timer_atk) {
    this->timer_atk = timer_atk;
}

void Player::setPoint(int point) {
    this->point = point;
}

// Functions
void Player::switchWeapon() {
    this->weapon = (this->weapon + 1) % this->weapons.size();
    this->wpn = Weapon(this->weapons[this->weapon]);
}

void Player::handleTimer_atk(int lastUpdate, int before) {
    if(isAttacking&& timer_atk== -1) {
        timer_atk = lastUpdate - before;
    }
    if(timer_atk != -1) {
        timer_atk = timer_atk + lastUpdate - before;
    }
    if(timer_atk >= getWpn().getAttackSpeed() + getWpn().getReload()) {
        timer_atk = -1;
    }
}