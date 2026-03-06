#include "ClientUI.hpp"

ClientUI::ClientUI(const sf::Clock clock, std::string name, sf::Color color) : Client(clock, name, color) {}

void ClientUI::drawGame() { // Game space
    const char* title = getName().c_str();

    ImGui::BeginChild(title, ImVec2(0, 400), true);

    ImVec2 childMin = ImGui::GetWindowPos();
    ImVec2 childMax = {
        childMin.x + ImGui::GetWindowSize().x,
        childMin.y + ImGui::GetWindowSize().y
    };

//        float deltaTime = clock.getElapsedTime().asMilliseconds() - lastUpdate;
//        std::cout << "TIME: " << clock.getElapsedTime().asSeconds() << "s // " << clock.getElapsedTime().asMilliseconds() << "ms" << std::endl;

    // ========= INPUT =========
    ImVec2 dir = {0.f, 0.f};

    Input inputs = getInputs();

    dir.x += 1.f * inputs.getMovementX();
    dir.y += 1.f * inputs.getMovementY();

    // ========= UPDATE =========
//        move(dir, deltaTime);

//        for (auto & [name, other] : opponents) {
//            Position p = resolveCollision(getPosition(), other.position);
//            other.position.setX(p.getX());
//            other.position.setY(p.getY());
//        }

        // ========= DRAW =========


        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        drawPlayer(draw_list, getPlayer(), childMin, childMax);

        for (auto & [name, other] : opponents) {
            drawPlayer(draw_list, other, childMin, childMax);
        }

    // PREDICTION
//    if (true) {
//        this->setPosition(smoothenDeplacement(getPosition(), dir, lastUpdate, clock.getElapsedTime().asMilliseconds()));
//    }

    lastUpdate = clock.getElapsedTime().asMilliseconds();
    ImGui::EndChild();
}


void ClientUI::addOpponent(const std::string& name, sf::Color color) {
    opponents.insert(std::make_pair(name, Player(0,name, color, Position())));
}

void ClientUI::drawConfig() {
    const char* title = getName().c_str();

    ImGui::BeginChild(title, ImVec2(0, 100), true); // hauteur FIXE
    ImGui::Text("%s", title);
    ImGui::Separator();

    int packetLoss = getPacketLoss();
    int ping = getPing();

    ImGui::SliderInt("Packet loss", &packetLoss, 0, 100);
    ImGui::InputInt("Ping", &ping);

    setPacketLoss(packetLoss);
    setPing(ping);

    ImGui::EndChild();
}