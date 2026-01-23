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
    // TODO: Re-add compensations
    // const char* modes[] = { "COMPO 1 ", "COMPO 2", "COMPO 3", "MODE 1", "MODE 2" };
    // int current = static_cast<int>(config.compensation);

    // ImGui::Combo("Compensation", &current, modes, IM_ARRAYSIZE(modes));
    // config.compensation = static_cast<NetConfig::CompensationMode>(current);

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
    ImGui::EndChild();
}