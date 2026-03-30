#ifndef PROJETM1_CONSOLE_HPP
#define PROJETM1_CONSOLE_HPP

#include <imgui.h>
#include <unordered_map>
#include "../src/implot.h"
#include "../Utils.hpp"

struct Packet {
    int timestampFrom;
    int timestampTo;
    short type;
    unsigned short from;
    unsigned short to = 0;
    bool wasReceived = false;
};

class Console {
private:
    int timestampDelay = 0;
    std::unordered_map<unsigned int, Packet> packets; // {timestamp: Packet}
    std::unordered_map<unsigned short, float> clients; // {PORT: Position}

public:
    Console();
    void addClient(unsigned short portToAdd);
    void addPacket(int timestamp, short type, unsigned short client = 0, int receivedAt = 0);

    void draw();
};


#endif //PROJETM1_CONSOLE_HPP
