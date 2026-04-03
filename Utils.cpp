#include "Utils.hpp"

sf::Color convertImUToSfColor(ImU32 im_color) {
    uint8_t r = (im_color >> 0) & 0xFF;
    uint8_t g = (im_color >> 8) & 0xFF;
    uint8_t b = (im_color >> 16) & 0xFF;
    uint8_t a = (im_color >> 24) & 0xFF;

    return {r,g,b,a};
}

uint32_t getPacketId() {
    m.lock();
    uint32_t val = packetID;
    packetID += 1;
    m.unlock();
    return val;
}

// TODO : add something to delete the global map texture at the end
std::vector<sf::Texture> GLOBAL_MAP_TEXTURES;
void loadGlobalMaps() {
    GLOBAL_MAP_TEXTURES.resize(Const::MAP_LINK.size());
    for (size_t i = 0; i < Const::MAP_LINK.size(); ++i) {
        if (!GLOBAL_MAP_TEXTURES[i].loadFromFile(Const::MAP_LINK[i])) {
            std::cout << "Failed to load map: " << Const::MAP_LINK[i] << std::endl;
        }
    }
}