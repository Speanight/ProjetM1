/*
 *Basic test Code
 *
#include <iostream>
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
*/
/*
 *Code SFLM - vérif
*
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

int main()
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML window");

    // Load a sprite to display
    const sf::Texture texture("../cute_image.png");
    sf::Sprite sprite(texture);

    // Create a graphical text to display
    const sf::Font font("../PlayfulTime-BLBB8.ttf");
    sf::Text text(font, "Hello SFML", 50);

    // Load a music to play
    sf::Music music("../wd-test.ogg");

    // Play the music
    music.play();

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Clear screen
        window.clear();


        // Draw the sprite
        window.draw(sprite);

        // Draw the string
        window.draw(text);

        // Update the window
        window.setVerticalSyncEnabled(false);
        window.setFramerateLimit(60);
        window.display();
    }
}
*/


/*
 * SFML 3.0 + ImGui minimal test
*/

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include <imgui.h>
#include "backends/imgui_impl_opengl3.h"

#include <cstring>
#include <iostream>


struct NetConfig {
    // Default configuration
    int packetLossGame1 = 0;
    int pingGame1 = 50;

    int packetLossGame2 = 0;
    int pingGame2 = 50;

    int tickrate = 60;

    enum class CompensationMode {
        COMPO_1 , COMPO_2, COMPO_3, MODE_1, MODE_2, None
    } compensation = CompensationMode::None;
};

const char* toString(NetConfig::CompensationMode mode)
{
    switch (mode)
    {
        case NetConfig::CompensationMode::None:    return "None";
        case NetConfig::CompensationMode::COMPO_1: return "COMPO_1";
        case NetConfig::CompensationMode::COMPO_2: return "COMPO_2";
        case NetConfig::CompensationMode::COMPO_3: return "COMPO_3";
        case NetConfig::CompensationMode::MODE_1:  return "MODE_1";
        case NetConfig::CompensationMode::MODE_2:  return "MODE_2";
        default: return "Unknown";
    }
}


void onNetConfigChanged(const NetConfig& config){
    std::cout << "Changement dans la config\n";

    std::cout << "Game 1:\n";
    std::cout << "  Packet loss: " << config.packetLossGame1 << "%\n";
    std::cout << "  Ping: " << config.pingGame1 << " ms\n";

    std::cout << "Game 2:\n";
    std::cout << "  Packet loss: " << config.packetLossGame2 << "%\n";
    std::cout << "  Ping: " << config.pingGame2 << " ms\n";

    std::cout << "Global:\n";
    std::cout << "  Tickrate: " << config.tickrate << "\n";
    std::cout << "  Compensation: "
              << toString(config.compensation) << "\n";

    std::cout << "=============================\n\n";
}



void drawGameZone(const char* title) {
    ImGui::BeginChild(title, ImVec2(0, 400), true);
    ImGui::Text("%s", title);
    ImGui::Separator();
    ImGui::Text("GAME COMING BEFORE GTA VI ! DW ! ");
    ImGui::EndChild();
}

bool drawGameConfig(const char* title, int& packetLoss, int& ping) {
    bool changed = false;

    ImGui::BeginChild(title, ImVec2(0, 100), true); // hauteur FIXE
    ImGui::Text("%s", title);
    ImGui::Separator();

    changed |= ImGui::SliderInt("Perte paquet", &packetLoss, 0, 100);
    changed |= ImGui::InputInt("Ping", &ping);

    ImGui::EndChild();
    return changed;
}

bool drawGlobalConfig(NetConfig& config) {
    bool changed = false;

    const char* modes[] = { "COMPO 1 ", "COMPO 2", "COMPO 3", "MODE 1", "MODE 2" };
    int current = static_cast<int>(config.compensation);

    changed |= ImGui::Combo("Compensation", &current, modes, IM_ARRAYSIZE(modes));
    config.compensation = static_cast<NetConfig::CompensationMode>(current);

    changed |= ImGui::InputInt("Tickrate", &config.tickrate);

    return changed;
}

void drawServerZone() {
    ImGui::BeginChild("Server", ImVec2(0, 0), true);
    ImGui::Text("SERVER");
    ImGui::Separator();
    ImGui::TextWrapped("BEHHHHHHHH, je sait pas moi, je suis un serveur ?! vous voulez une bierre ? un café");
    ImGui::EndChild();
}

void drawMainUI(NetConfig& config) {
    static NetConfig last = config;
    bool changed = false;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("Main", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar
    );

    ImGui::Columns(2);
    drawGameZone("CLIENT A");
    ImGui::NextColumn();
    drawGameZone("CLIENT B");
    // ImGui::Columns(1);

    ImGui::Separator();

    ImGui::Columns(2, nullptr, false);

    changed |= drawGameConfig(
        "CLIENT A - config",
        config.packetLossGame1,
        config.pingGame1
    );

    ImGui::NextColumn();

    changed |= drawGameConfig(
        "CLIENT B - config",
        config.packetLossGame2,
        config.pingGame2
    );

    ImGui::Columns(1);


    ImGui::Separator();

    changed |= drawGlobalConfig(config);

    ImGui::Separator();

    drawServerZone();

    ImGui::End();

    if (changed && memcmp(&last, &config, sizeof(NetConfig)) != 0)
    {
        onNetConfigChanged(config);
        last = config;
    }
}

int main() {
    // Print window
    sf::ContextSettings settings;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    sf::Window window(
        sf::VideoMode({800, 900}),
        "Pourquoi frag avec 1000 de ping c'est chiant",
        sf::State::Windowed,
        settings
    );

    window.setVerticalSyncEnabled(true);

    // check imgui OK
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 130");

    sf::Clock clock;
    NetConfig config;

    // thread of window
    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = clock.restart().asSeconds();

        auto size = window.getSize();
        io.DisplaySize = ImVec2((float)size.x, (float)size.y);

        auto mouse = sf::Mouse::getPosition(window);
        io.MousePos = ImVec2((float)mouse.x, (float)mouse.y);
        io.MouseDown[0] = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        drawMainUI(config);

        ImGui::Render();

        glClearColor(0.15f, 0.15f, 0.15f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.display();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
