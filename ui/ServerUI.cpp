#include <iostream>
#include "ServerUI.hpp"

ServerUI::ServerUI(Console& console) : console(console) {
    std::vector<float> vec(Const::GRAPH_DISPLAY_VALUES, FLT_EPSILON);
    data["Server"] = vec;

    lastTimestamp = 0;
}

void ServerUI::refreshServerUI() {
    clients.clear();
    lines.clear();
    data.clear();
    console.refreshConsole();

    // resseting the server datas
    std::vector<float> vec(Const::GRAPH_DISPLAY_VALUES, FLT_EPSILON);
    data["Server"] = vec;
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
    if (pauseConsole) return;

    std::lock_guard<std::mutex> lock(m);

    int dt = timestamp - lastTimestamp;
    int toPush = std::max(1, dt / Const::GRAPH_DISPLAY_MS);

    for (auto& [name, vec] : data) {
        vec.insert(vec.end(), toPush, 0.0f);

        if (vec.size() > Const::GRAPH_DISPLAY_VALUES) {
            vec.erase(vec.begin(), vec.begin() + (vec.size() - Const::GRAPH_DISPLAY_VALUES));
        }
    }

    auto& v = data[from];
    if (!v.empty()) {
        v.back() += 100.0f;
    }

    lastTimestamp = timestamp;
}

void ServerUI::draw() {
    //////////////
    // SETTINGS //
    //////////////

    ImGui::Text("SERVER | Tickrate: ");
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

    if (ImGui::BeginTable("MainLayout", 2, ImGuiTableFlags_Resizable)) {
        ImGui::TableNextColumn();
        ImGui::BeginChild("LeftTop", ImVec2(0, ImGui::GetFrameHeightWithSpacing()), false);
        if (ImGui::Button("Pause", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            pauseConsole = !pauseConsole;
            console.setPause(pauseConsole);
        }
        ImGui::EndChild();
        ImGui::Separator();

        ImGui::BeginChild("console", ImVec2(0, 0), false);
        for (const auto& line : std::ranges::reverse_view(lines)) {
            ImGui::TextColored(line.color, "%s", line.text.c_str());
        }
        ImGui::EndChild();
        ImGui::TableNextColumn();
        ImGui::BeginChild("RightTop", ImVec2(0, ImGui::GetFrameHeightWithSpacing()), false);
        static int selectedGraph = 0; // default

        float width = ImGui::GetContentRegionAvail().x / 3.0f;

        if (ImGui::Button("Graphe 1", ImVec2(width, 0))) selectedGraph = 0;
        ImGui::SameLine();
        if (ImGui::Button("Graphe 2", ImVec2(width, 0))) selectedGraph = 1;
        ImGui::SameLine();
        if (ImGui::Button("Graphe 3", ImVec2(width, 0))) selectedGraph = 2;
        ImGui::EndChild();
        ImGui::Separator();

        /////////////
        //  GRAPH  //
        /////////////

        ImGui::BeginChild("GraphZone", ImVec2(0, 0), false);

        ImVec2 avail = ImGui::GetContentRegionAvail();
        float graphHeight = avail.y;

        // option : si tu veux laisser un peu de marge UI
        graphHeight -= 5.0f;
        graphHeight = std::max(graphHeight, 150.0f);

        if (selectedGraph == 0) {
            console.setPause(true);
            drawGraph1();
        } else if (selectedGraph == 1) {
            // Game zone
            console.setPause(pauseConsole);
            drawGame();

        } else if (selectedGraph == 2) {
            console.setPause(pauseConsole);
            ImGui::Text("Graphe 3");
            console.draw(graphHeight);
        }
        ImGui::EndChild(); // GraphZone
    }
    ImGui::EndTable();

    ImGui::EndChild();
}

// ============= GRAPH 1 =============
void ServerUI::drawGraph1() {
    if (ImGui::BeginTable("##table", 2)) {
        static int groups = Const::GRAPH_DISPLAY_VALUES; // dataTab of server should have 200 values (items*groups)

        // Table headers:
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 75.0f);
            ImGui::TableSetupColumn("Data traffic", ImGuiTableColumnFlags_WidthFixed, ImGui::GetContentRegionAvail().x - 75.0f);
        }

        // Server tab:
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Server");
            ImGui::TableSetColumnIndex(1);
            ImGui::PushID("server");

            if (ImPlot::BeginPlot("Bar Group##", ImVec2{ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / (3)},
                              ImPlotFlags_NoTitle | ImPlotFlags_NoLegend |
                              ImPlotFlags_NoMouseText | ImPlotFlags_NoFrame | ImPlotFlags_NoInputs)) {
                static const char* ilabels[] = {"Packets received", "Packets sent"};

                ImPlot::SetupAxes("Time","Packet",ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);

                ImPlot::PlotBarGroups(
                    ilabels,
                    data["Server"].data(),
                    1,
                    groups,
                    1,
                    0,
                    {ImPlotProp_Flags, ImPlotBarGroupsFlags_Stacked}
                    );

                ImPlot::EndPlot(); // Server's Bar Group
                }
            ImGui::PopID();
        }


        // CLients tab :
        int i = 1;
        {
            m.lock();
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
                        ImPlot::PlotBarGroups(
                            ilabels,
                            data[from].data(),
                            1,
                            groups,
                            1,
                            0,
                            {ImPlotProp_Flags, ImPlotBarGroupsFlags_Stacked}
                            );

                        ImPlot::EndPlot(); // Bar Group
                                      }
                    ImGui::PopID();
                    i++;
                }
            }
            m.unlock();
        }

        ImGui::EndTable();
    }
}


// ============= GRAPH 2 =============
/**
 * Set the player on the database of the serverUI (copy)
 * @param id        : port of the player to find it in the player list
 * @param player    : data of the player [color, position, weapon ...]
 */
void ServerUI::setPlayer(unsigned short id, Player player) {
    clients[id] = player;
}

/**
 * Update the client data based on the actual state contained in the server
 * @param id    : port of the player used as an ID
 * @param s     : state of the player in the server
 */
void ServerUI::updateClient(unsigned short id, State s) {
    auto it = clients.find(id);
    if (it == clients.end()) return; // security

    Player& player = it->second;

    player.setPosition(s.getPosition());
    player.setRadius(s.getRadius());
    player.setIsAttacking(s.getAttack());
    player.setPoint(s.getPoint());

    player.getWpn().applyID(s.getWpn().getId());
}

/**
 * Get the mapID and set it
 * @param mapID : id of the map to find it in the GLOBAL_MAP_TEXTURES
 */
void ServerUI::setMapID(int mapID) {
    this->mapID = mapID;
}

/**
 * Draw the graph (game area on the server)
 */
void ServerUI::drawGame() {
    ImVec2 avail = ImGui::GetContentRegionAvail();

    float size = std::min(avail.x, avail.y);
    size = std::max(size, 200.f); // MINIMUM GAME SIZE


    float offsetX = (avail.x - size) * 0.5f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

    ImGui::BeginChild("##game", ImVec2(size, size), true);

    ImVec2 childMin = ImGui::GetWindowPos();
    ImVec2 childMax = {
        childMin.x + size,
        childMin.y + size
    };
    // DRAW
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // drawErrorScreen(draw_list, childMin, childMax);
    if (!clients.empty()) {
        auto it = clients.begin();

        Player player = it->second;

        std::map<std::string, Player> opponents;
        ++it;

        for (; it != clients.end(); ++it) {
            const Player& p = it->second;

            std::string name = p.getName();
            opponents[name] = p;
        }

        drawFightingScreen(draw_list, player, opponents, childMin, childMax, mapID);
    }

    ImGui::EndChild();

}