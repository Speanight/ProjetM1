#ifndef PROJETM1_CONSOLE_HPP
#define PROJETM1_CONSOLE_HPP

#include <imgui.h>
#include <unordered_map>
#include <map>
#include <iostream>
#include <cmath>
#include <mutex>
#include "../src/implot.h"
#include "../Utils.hpp"

struct Packet {
    int timestampFrom;
    int timestampTo;
    short type;
    unsigned short from;
    unsigned short to;
    bool wasReceived = false;
};

class Console {
private:
    std::mutex m;
    int timestampDelay = 0;
    std::map<uint32_t, Packet> packets; // {id: Packet}
    std::map<unsigned short, int> clients; // {PORT: Position}
    bool pause = false;

public:
    Console();
    void setPause(bool pause);
    void addClient(unsigned short portToAdd);
    void addPacket(uint32_t id, short type, unsigned short client, int timestamp, bool received = false);

    void draw();
};


#endif //PROJETM1_CONSOLE_HPP
