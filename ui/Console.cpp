#include <iostream>
#include "Console.hpp"

Console::Console() = default;

void Console::addClient(unsigned short portToAdd) {
    clients[portToAdd] = clients.size()*100;
}


void Console::addPacket(int timestamp, short type, unsigned short client, int receivedAt) {
    std::cout << "Packet: from " << client << " @" << timestamp << " #" << type << " & received @" << receivedAt << std::endl;
    // TODO: fix and re-add a deleting function.
//    if (timestamp > timestampDelay + Const::GRAPH_DISPLAY_VALUES) {
//        timestampDelay = timestamp - Const::GRAPH_DISPLAY_VALUES;
//        std::erase_if(packets, [this](const auto& item) {
//            const auto& [key, value] = item;
//            return key < timestampDelay + Const::GRAPH_DISPLAY_VALUES;
//        });
//    }

    Packet p;

    if (receivedAt != 0) {
        p = packets[timestamp];
        p.to = client;
        p.timestampTo = receivedAt;
        p.wasReceived = true;

        packets[timestamp] = p;
    }
    else {
        p.timestampFrom = timestamp;
        p.type = type;
        p.from = client;

        // Here to avoid empty values (0) that would display a long line:
        p.timestampTo = timestamp;
        p.to = client;

        packets[timestamp] = p;
    }
}


void Console::draw() {
    static ImPlotSpec spec(ImPlotProp_Marker, ImPlotMarker_Auto);
    spec.MarkerSize = 5.f;
    spec.LineWeight = 2.f;
    spec.FillAlpha = 1.f;



    if (ImPlot::BeginPlot("##MarkerStyles", ImVec2(-1,0), ImPlotFlags_CanvasOnly)) {
//        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
        ImPlot::SetupAxesLimits(0, Const::GRAPH_DISPLAY_VALUES, 0, 100);

        // filled markers
        for (auto & [timestamp, packet] : packets) {
            // Get x/y values depending of available region:
            int x[2] = {static_cast<int>(Const::GRAPH_DISPLAY_VALUES * packet.timestampFrom - timestampDelay),
                        static_cast<int>(Const::GRAPH_DISPLAY_VALUES * packet.timestampTo - timestampDelay)};
            int y[2] = {static_cast<int>(clients[packet.from]), static_cast<int>(clients[packet.to])};


            ImGui::PushID(packet.type); // Change line display depending on packet type.
            ImPlot::PlotLine("##Filled", x, y, 1+packet.wasReceived, spec); // Draw values
            ImGui::PopID();
        }

        ImPlot::EndPlot();
    }
}
// TODO: remove all u.int / int / short conversions because values are wrong.