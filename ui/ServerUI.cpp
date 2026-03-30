#include <iostream>
#include "ServerUI.hpp"

ServerUI::ServerUI() {
    std::vector<float> vec(Const::GRAPH_DISPLAY_VALUES, FLT_EPSILON);
    data["Server"] = vec;

    lastTimestamp = 0;
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

void ServerUI::addToData(const std::string& to) {
    std::vector<float> vec(Const::GRAPH_DISPLAY_VALUES, FLT_EPSILON);
    data[to] = vec;
}

void ServerUI::removeToData(const std::string& to) {
    data.erase(to);
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
    if (!pauseConsole) {
        int toPush = (timestamp - lastTimestamp) / Const::GRAPH_DISPLAY_MS; // amt. of values to "push"

        // If we're in to the next displayed bar:
        for (auto & [f, t] : data) {
                data[f].insert(data[f].end(), toPush, FLT_EPSILON);

            if (data[f].size() > Const::GRAPH_DISPLAY_VALUES) {
                data[f].erase(data[f].begin(), data[f].begin() + toPush + 1);
            }
        }

        // Finally, we insert the value in last place:
        data[from].push_back(100.0f);
        lastTimestamp = timestamp;
    }
}

void ServerUI::draw() {
    //////////////
    // SETTINGS //
    //////////////

    ImGui::Text("Tickrate: ");
    ImGui::SameLine();
    int tick = tickrate;
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
    if (ImGui::InputInt("##tickRate", &tick, 1, 100)) {
        // Check value is correct:
        if (tick < 1) {
            tick = 1;
        }
        if (tick > 100) {
            tick = 100;
        }
        tickrate = tick;
    }
    ImGui::SameLine();
    ImGui::Text("Client send rate: ");
    ImGui::SameLine();
    tick = clientRefreshRate;
    if (ImGui::InputInt("##clientSendRate", &tick, 1, 250)) {
        // Check value is correct:
        if (tick < 1) {
            tick = 1;
        }
        if (tick > 250) {
            tick = 250;
        }
        clientRefreshRate = tick;
    }
    ImGui::SameLine();
    ImGui::Checkbox("Server Rewind (hit validation)", &rewind);
    ImGui::Separator();

    /////////////
    // CONSOLE //
    /////////////

    ImGui::BeginChild("Server", ImVec2(0, 0), true);
    ImGui::Text("SERVER");
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        pauseConsole = !pauseConsole;
    }
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::BeginChild("console", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y));
    for (const auto& line : std::ranges::reverse_view(lines)) {
        ImGui::TextColored(line.color, "%s", line.text.c_str());
    }
    ImGui::EndChild(); // "console" child

    /////////////
    //  GRAPH  //
    /////////////
    ImGui::SameLine();


    // TODO: new plot: displays packets in real time:
    static ImPlotSpec spec(ImPlotProp_Marker, ImPlotMarker_Auto);
    spec.MarkerSize = 5.f;
    spec.LineWeight = 2.f;

//    if (ImPlot::BeginPlot("##MarkerStyles", ImVec2(-1,0), ImPlotFlags_CanvasOnly)) {
//
//        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
//        ImPlot::SetupAxesLimits(0, 10, 0, 12);
//
//        int xs[2] = {1,4};
//        int ys[2] = {10,11};
//
//        // filled markers
//        for (int m = 0; m < ImPlotMarker_COUNT; ++m) {
//            ImGui::PushID(m);
//            spec.FillAlpha = 1.0f;
//            ImPlot::PlotLine("##Filled", xs, ys, 2, spec);
////            ImPlot::PlotLine("##Filled", {1}, {10}, 2, spec);
//            ImGui::PopID();
//            ys[0]--; ys[1]--;
//        }
//
//        ImPlot::PushStyleColor(ImPlotCol_InlayText, ImVec4(1,0,1,1));
//        ImPlot::PlotText("Vertical Text", 5.0f, 6.0f, ImVec2(0,0), {ImPlotProp_Flags, ImPlotTextFlags_Vertical});
//        ImPlot::PopStyleColor();
//
//        ImPlot::EndPlot();
//    }

    console.draw();


    /* // OLD PLOT: DISPLAYS PACKETS RECEIVED/SENT TO SERVER
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

        static int groups = Const::GRAPH_DISPLAY_VALUES; // dataTab of server should have 200 values (items*groups)

        if (ImPlot::BeginPlot("Bar Group##", ImVec2{ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / (3)},
                              ImPlotFlags_NoTitle | ImPlotFlags_NoLegend |
                              ImPlotFlags_NoMouseText | ImPlotFlags_NoFrame | ImPlotFlags_NoInputs)) {
            static const char* ilabels[] = {"Packets received", "Packets sent"};

            ImPlot::SetupAxes("Time","Packet",ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
            ImPlot::PlotBarGroups(ilabels,data["Server"].data(),1,groups,1,0,{ImPlotProp_Flags, ImPlotBarGroupsFlags_Stacked});

            ImPlot::EndPlot(); // Server's Bar Group

        }
        ImGui::PopID();

        int i = 1;

        for (auto & [from, values] : data) {
            if (from != "Server") {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", from.c_str());
                ImGui::TableSetColumnIndex(1);

                // Prints data:
                ImGui::PushID(i);
                if (ImPlot::BeginPlot("Bar Group##", ImVec2{ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / (data.size()-i)},
                                      ImPlotFlags_NoTitle | ImPlotFlags_NoLegend |
                                      ImPlotFlags_NoMouseText | ImPlotFlags_NoFrame | ImPlotFlags_NoInputs)) {
                    static const char* ilabels[] = {"Packet Loss", "Packet Delivered"};

                    ImPlot::SetupAxes("Time","Packet",ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
                    ImPlot::PlotBarGroups(ilabels,data[from].data(),1,groups,1,0,{ImPlotProp_Flags, ImPlotBarGroupsFlags_Stacked});

                    ImPlot::EndPlot(); // Bar Group
                }
                ImGui::PopID();
                i++;
            }
        }
        ImGui::EndTable();
    }
    */

    ImGui::EndChild();
}