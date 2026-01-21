#include "MainWindow.hpp"

MainWindow::MainWindow() {}

void MainWindow::draw() {
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
    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);

    drawGameConfig(
            "CLIENT A - config",
            config.packetLossGame1,
            config.pingGame1
    );

    ImGui::NextColumn();

    drawGameConfig(
            "CLIENT B - config",
            config.packetLossGame2,
            config.pingGame2
    );

    ImGui::Columns(1);
    ImGui::Separator();
    changed |= drawGlobalConfig(config);
    ImGui::Separator();
    server.draw();
    ImGui::End();

    if (changed && memcmp(&last, &config, sizeof(NetConfig)) != 0) {
        onNetConfigChanged(config);
        last = config;
    }
}

void MainWindow::loop() {
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Projet M1");

    // check imgui OK
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui::SFML::Init(window);

    sf::Clock deltaClock;
    sf::Time delta;
//    NetConfig config;

    // thread of window
    while (window.isOpen())
    {
        delta = deltaClock.restart();
        while (auto event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = deltaClock.restart().asSeconds();

        auto size = window.getSize();
        io.DisplaySize = ImVec2((float)size.x, (float)size.y);

        auto mouse = sf::Mouse::getPosition(window);
        io.MousePos = ImVec2((float)mouse.x, (float)mouse.y);
        io.MouseDown[0] = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

        ImGui::SFML::Update(window, delta);

        window.clear();
        draw();

        glClearColor(0.15f, 0.15f, 0.15f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        window.setView(window.getDefaultView());

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    ImGui::DestroyContext();

}