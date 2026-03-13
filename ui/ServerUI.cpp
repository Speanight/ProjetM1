#include <zconf.h>
#include "ServerUI.hpp"

ServerUI::ServerUI() {
    std::vector<float> vec(Const::GRAPH_DISPLAY_VALUES, FLT_EPSILON);
    data["Server"];
    data["Server"]["Server"] = vec;
}

/**
 * Adds a line to the Server's console.
 *
 * @param text Text to be added
 * @param color Color to display the text in.
 */
void ServerUI::addLine(std::string text, sf::Color color) {
    if (!pauseConsole) {
        ConsoleLine cl;
        cl.color = color;
        cl.text = std::move(text);
        lines.push_back(cl);

        // Removes the oldest line if limit (set in consts) is reached.
        if (lines.size() > Const::CONSOLE_LINES) {
            lines.erase(lines.begin());
        }
    }
}

void ServerUI::addToData(const std::string& from, const std::string& to) {
    std::vector<float> vec(Const::GRAPH_DISPLAY_VALUES, FLT_EPSILON);
    data[from];
    data[from][to] = vec;
    data[to];
    data[to][from] = vec;
}

void ServerUI::addLine(int timestamp, std::string from, std::string to, std::string details, sf::Color color) {
    if (!pauseConsole) {
        // Generate the console line:
        ConsoleLine cl;
        cl.color = color;
        cl.text = from + " >>> " + to + " | " + details;
        lines.push_back(cl);

        // Removes oldest line if limit is reached. Limit is set in consts.
        if (lines.size() > Const::CONSOLE_LINES) {
            lines.erase(lines.begin());
        }
    }
}

void ServerUI::addToGraph(int timestamp, const std::string& from, const std::string& to) {
    // TODO: Find a better way to do it (way too slow: causes cumulative delay) - seems to only work for "some values" also.
//    if (!pauseConsole) {
//        // If we're in to the next displayed bar:
//        while (timestamp - Const::GRAPH_DISPLAY_MS > this->lastTimestamp) {
//            for (auto & [f, t] : data) {
//                for (auto &[name, values] : data[f]) {
//                    data[f][name].pop_back();
//                    data[f][name].insert(data[f][name].begin(), FLT_EPSILON);
//                }
//            }
//            timestamp -= Const::GRAPH_DISPLAY_MS;
//        }
//
//        // Finally, we insert the value in last place:
//        data[from][to].at(0) = 1.0f;
//    }
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
    ImGui::SameLine();
    ImGui::Checkbox("Pause", &this->pauseConsole);
    ImGui::Separator();
    ImGui::BeginChild("console", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y));
    for (const auto& line : std::ranges::reverse_view(lines)) {
        ImGui::TextColored(line.color, "%s", line.text.c_str());
    }
    ImGui::EndChild(); // "console" child

    // TODO: Create graph to visualize sending/reception of data from server.
    // Resource (used here, in the comments): https://github.com/epezent/implot?tab=readme-ov-file
    /////////////
    //  GRAPH  //
    /////////////
    ImGui::SameLine();
    if (ImGui::BeginTable("##table", 2)) {
        // Table headers:
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Data traffic", ImGuiTableColumnFlags_WidthFixed, ImGui::GetContentRegionAvail().x - 75.0f);
        ImGui::TableHeadersRow();

        // Server tab:
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Server");
        ImGui::TableSetColumnIndex(1);
        ImGui::PushID("server");
        ImPlot::PushColormap(serverColorMap);

        std::vector<float> dataGraph;
        for (auto & [name, value] : data["Server"]) {
            dataGraph.insert(dataGraph.end(), value.begin(), value.end());
        }

        if (ImPlot::BeginPlot("Bar Group##", ImVec2{ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / (3)},
                              ImPlotFlags_NoTitle | ImPlotFlags_NoLegend |
                              ImPlotFlags_NoMouseText | ImPlotFlags_NoFrame | ImPlotFlags_NoInputs)) {
            static const char* ilabels[] = {"Packets received", "Packets sent"};
            static int items = data["Server"].size();
            static int groups = Const::GRAPH_DISPLAY_VALUES; // dataTab of server should have 200 values (items*groups)

            ImPlot::SetupAxes("Time","Packet",ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
            ImPlot::PlotBarGroups(ilabels,dataGraph.data(),items,groups,(float)Const::GRAPH_DISPLAY_VALUES/50,0,{ImPlotProp_Flags, ImPlotBarGroupsFlags_Stacked});

            ImPlot::EndPlot(); // Server's Bar Group

        }
        ImGui::PopID();

        for (int i = 0; i < 3; i++) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Testing %d", i);
            ImGui::TableSetColumnIndex(1);

            // Prints data:
            ImGui::PushID(i);
            ImPlot::PushColormap(clientsColorMap);
            if (ImPlot::BeginPlot("Bar Group##", ImVec2{ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / (3-i)},
                                  ImPlotFlags_NoTitle | ImPlotFlags_NoLegend |
                                  ImPlotFlags_NoMouseText | ImPlotFlags_NoFrame | ImPlotFlags_NoInputs)) {
                static const char* ilabels[] = {"Packet Loss", "Packet Delivered"};
                static float dataTab[100] = {1, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON,1, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON,1, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON,1, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON,1, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON,  // Packet Loss
                                             FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON,FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON,FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON,FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON,FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON, FLT_EPSILON, 1, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON,}; // Packet Delivered
                static int items = 2;
                static int groups = 50;

                ImPlot::SetupAxes("Time","Packet",ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
                ImPlot::PlotBarGroups(ilabels,dataTab,items,groups,1,0,{ImPlotProp_Flags, ImPlotBarGroupsFlags_Stacked});

                ImPlot::EndPlot(); // Bar Group
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::EndChild();
}

void ServerUI::setColorMaps(ImU32 server[], ImU32 client[]) {
    this->serverColorMap = ImPlot::AddColormap("Server packets colors", server, 2);
    this->clientsColorMap = ImPlot::AddColormap("Client packets colors", client, 2);
}