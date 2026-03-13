#include "ServerUI.hpp"

ServerUI::ServerUI() {}

/**
 * Adds a line to the Server's console.
 *
 * @param text Text to be added
 * @param color Color to display the text in.
 */
void ServerUI::addLine(std::string text, sf::Color color) {
    ConsoleLine cl;
    cl.color = color;
    cl.text = std::move(text);
    lines.push_back(cl);

    // Removes the oldest line if limit (set in consts) is reached.
    if (lines.size() > Const::CONSOLE_LINES) {
        lines.erase(lines.begin());
    }
}

void ServerUI::draw() {
    //////////////
    // SETTINGS //
    //////////////

    // .count converts it to a long value. 1000/ticks gives tickrate, and convert it back to an int to avoid digits.
    ImGui::Text("Tickrate: %d", int(1000 / Const::TICKRATE.count()));
    ImGui::Separator();

    /////////////
    // CONSOLE //
    /////////////
    ImGui::BeginChild("Server", ImVec2(0, 0), true);
    ImGui::Text("SERVER");
    ImGui::Separator();
    for (const auto& line : std::ranges::reverse_view(lines)) {
        ImGui::TextColored(line.color, "%s", line.text.c_str());
    }

    // TODO: Create graph to visualize sending/reception of data from server.
    // Resource (used here, in the comments): https://github.com/epezent/implot?tab=readme-ov-file
    /////////////
    //  GRAPH  //
    /////////////
//    ImGui::SameLine();
//    if (ImGui::BeginTable("##table", 2)) {
//        ImGui::TableSetupColumn("Name");
//        ImGui::TableSetupColumn("Data transfer in time");
//        ImGui::TableHeadersRow();
//        ImPlot::PushColormap(ImPlotColormap_Cool);
//        for (int row = 0; row < 10; row++) {
//            ImGui::TableNextRow();
//            static float data[100];
//            srand(row);
//            for (int i = 0; i < 100; ++i)
//                data[i] = RandomRange(0.0f,10.0f);
//            ImGui::TableSetColumnIndex(0);
//            ImGui::Text("EMG %d", row);
//            ImGui::TableSetColumnIndex(1);
//            ImGui::PushID(row);
//            MyImPlot::Sparkline("##spark",data,100,0,11.0f,offset,ImPlot::GetColormapColor(row),ImVec2(-1, 35));
//            ImGui::PopID();
//        }
//        ImPlot::PopColormap();
//        ImGui::EndTable();
//    }

    ImGui::EndChild();
}