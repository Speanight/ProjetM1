#include "Utils.hpp"

sf::Color convertImUToSfColor(ImU32 im_color) {
    uint8_t r = (im_color >> 0) & 0xFF;
    uint8_t g = (im_color >> 8) & 0xFF;
    uint8_t b = (im_color >> 16) & 0xFF;
    uint8_t a = (im_color >> 24) & 0xFF;

    return {r,g,b,a};
}