#include "Console.hpp"

Console::Console() {
    clients.insert({Const::COMM_PORT_SERVER, 50});
}

void Console::setPause(bool pause) {
    this->pause = pause;
}

void Console::addClient(unsigned short portToAdd) {
    if (portToAdd != Const::COMM_PORT_SERVER) {
        int pos = 50 / ceil(clients.size()/2);
        if (clients.size() % 2) {
            pos = -pos;
        }
        pos += 50;
        clients.insert({portToAdd, pos});
    }
}

void Console::addPacket(uint32_t id, short type, unsigned short client, int timestamp, bool received) {
    // TODO: fix and re-add a deleting function.
    if (pause) {
        return;
    }
    m.lock();
    if (packets.size() > Const::GRAPH_DISPLAY_VALUES) {
        auto it = packets.begin();
        // Remove X first element to have GRAPH_DISPLAY_VALUES elems. left.
        std::advance(it, packets.size() - Const::GRAPH_DISPLAY_VALUES);

        try {
            packets.erase(packets.begin(), it);
        } catch (int errCode) {
            std::cout << "Couldn't clear packets from console! Error: " << errCode;
        }
    }
    timestampDelay = packets.begin()->second.timestampFrom;

    Packet p;

    if (packets.contains(id)) {
        p = packets[id];
        p.wasReceived = true;
    }

    if (received) {
        p.to = client;
        p.timestampTo = timestamp;
    }
    else {
        p.from = client;
        p.timestampFrom = timestamp;
    }
    p.type = type;

    packets[id] = p;
    m.unlock();
}

void Console::refreshConsole() {
    clients.clear();
    packets.clear();

    clients.insert({Const::COMM_PORT_SERVER, 50});
}


void Console::draw() {
    static ImPlotSpec spec(ImPlotProp_Marker, ImPlotMarker_Auto);
    spec.MarkerSize = 5.f;
    spec.LineWeight = 2.f;
    spec.FillAlpha = 1.f;

    if (ImPlot::BeginPlot("##MarkerStyles", ImVec2(-1,0), ImPlotFlags_CanvasOnly)) {
//        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
        ImPlot::SetupAxesLimits(0, Const::GRAPH_DISPLAY_VALUES, 0, 100);
        ImPlot::SetupAxisZoomConstraints(ImAxis_Y1, 105, 105);

        m.lock();
        int firstTimestamp = packets.begin()->second.timestampFrom;
        for (auto & [id, packet] : packets) {
            // Get x/y values depending of available region:
            int x[2] = {static_cast<int>(packet.timestampFrom - firstTimestamp),
                        static_cast<int>(packet.timestampTo - firstTimestamp)};
            int y[2] = {clients[packet.from], clients[packet.to]};


            ImGui::PushID(packet.type); // Change line display depending on packet type.
            ImPlot::PlotLine("##Filled", x, y, 1+packet.wasReceived, spec); // Draw values
            ImGui::PopID();
        }
        m.unlock();

        ImPlot::EndPlot();
    }
}