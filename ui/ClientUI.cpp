#include "ClientUI.hpp"

#include <algorithm>

ClientUI::ClientUI(const sf::Clock clock, std::string name, sf::Color color) : Client(clock, name, color) {}

void ClientUI::drawGame() { // Game space
    const char* title = getName().c_str();

    // calculating the window so it's always a square
    // Max available size from parent window
    ImVec2 avail = ImGui::GetContentRegionAvail();

    // game size
    float size = std::max(std::min(avail.x, avail.y - 300.f), 400.f); //TODO : replace the 500.f by a constant that is the size of the server UI

    // minimal game size
    size = std::max(size, 400.f);

    // center horizontally only (top aligned)
    float offsetX = (avail.x - size) * 0.5f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
    // no Y offset -> stays at top

    // insert the game space
    ImGui::BeginChild(title, ImVec2(size, size), true);

    ImVec2 childMin = ImGui::GetWindowPos();
    ImVec2 childMax = {
        childMin.x + size,
        childMin.y + size
    };

    // ========= INPUT =========
    ImVec2 dir = {0.f, 0.f};

    Input inputs = getInputs();

    dir.x += 1.f * inputs.getMovementX();
    dir.y += 1.f * inputs.getMovementY();

    // ========= DRAW =========
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    drawPlayer(draw_list, getPlayer(), childMin, childMax);

    for (auto & [name, other] : opponents) {
        drawPlayer(draw_list, other, childMin, childMax);
    }

    // ======== COMPENSATIONS =========
    // Prediction
    if (this->getCompensations()[Compensation::PREDICTION]) {
        // TODO: Prediction with clock to avoid desync issues.
        Position pos;
        int now = clock.getElapsedTime().asMilliseconds();
        pos.setX(getPlayer().position.getX() + inputs.getMovementX() * Const::PLAYER_SPEED * (now - lastUpdate));
        pos.setY(getPlayer().position.getY() + inputs.getMovementY() * Const::PLAYER_SPEED * (now - lastUpdate));
        setPosition(pos);
        setRadius(getPlayer().radius + inputs.getRotate() * Const::PLAYER_RADIUS_SPEED * (now - lastUpdate));
    }

    // Interpolation
    if (this->getCompensations()[Compensation::INTERPOLATION]) {
        std::unordered_map<std::string, State> pastState = bufferOnReceipt.getTState(-1);
        std::unordered_map<std::string, State> currState = bufferOnReceipt.getCurrentState();
        for (auto & [name, other] : opponents) {
            if (name != getName() and name != "") {
                Position pastPos = pastState[name].getPosition();
                Position currPos = currState[name].getPosition();

                // Position = old one + diff. * (0 at beginning of tick, 1 at end of tick)
                double tickProgress = (clock.getElapsedTime().asMilliseconds() - lastServerTick) / (double)Const::TICKRATE.count();
                Position pos;

                pos.setX(pastPos.getX() + (currPos.getX() - pastPos.getX()) * tickProgress);
                pos.setY(pastPos.getY() + (currPos.getY() - pastPos.getY()) * tickProgress);

                // TODO: Semaphore to read old buffer (or segfault).
                opponents[name].position = pos;

                // If the radius goes through 0, make sure we rotate correctly.
                if (abs(pastState[name].getRadius() - currState[name].getRadius()) > M_2_PI) {
                    opponents[name].radius = pastState[name].getRadius() + (currState[name].getRadius() - pastState[name].getRadius()) * tickProgress;
                }
                else {
                    opponents[name].radius = pastState[name].getRadius() + (currState[name].getRadius() - pastState[name].getRadius()) * tickProgress;
                }
            }
        }
    }

    // TODO: Reconciliation
    if (this->getCompensations()[Compensation::RECONCILIATION]) {
    }

    lastUpdate = clock.getElapsedTime().asMilliseconds();
    ImGui::EndChild();
}


void ClientUI::addOpponent(const std::string& name, sf::Color color) {
    opponents.insert(std::make_pair(name, Player(0,name, color, Position())));
    this->bufferOnReceipt.addClient(Player(0,name,color,Position()));
}

/**
 * Draws the configuration that shows at the top of the player's screen. It is used to
 * change ping and packet loss values, as well as change compensation enabled/disabled.
 */
void ClientUI::drawConfig() {
    const char* title = getName().c_str();

    ImGui::BeginChild(title, ImVec2(0, 100), true); // hauteur FIXE
    ImGui::Text("%s", title);
    ImGui::Separator();

    int packetLoss = getPacketLoss();
    int ping = getPing();
    std::unordered_map<int,bool> compensations = getCompensations();

    ImGui::SliderInt("Packet loss", &packetLoss, 0, 100);
    ImGui::InputInt("Ping", &ping);

    ImGui::Checkbox("Interpolation", &compensations[Compensation::INTERPOLATION]);
    ImGui::SameLine();
    ImGui::Checkbox("Prediction", &compensations[Compensation::PREDICTION]);
    ImGui::SameLine();
    ImGui::Checkbox("Reconciliation", &compensations[Compensation::RECONCILIATION]);

    setPacketLoss(packetLoss);
    setPing(ping);

    setCompensations(compensations);

    ImGui::EndChild();
}