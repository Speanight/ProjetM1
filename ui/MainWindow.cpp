#include "MainWindow.hpp"

MainWindow::MainWindow(sf::Clock clock) : server(clock) {
    thread = std::thread(&MainWindow::loop, this);
}

MainWindow::~MainWindow() {
    if (thread.joinable()) {
        thread.join();
    }
}

void MainWindow::addClient(ClientUI* client) {
    clients.push_back(client);
    server.addClient(client->init());
}

/**
 * Draw the main window
 */
void MainWindow::draw() {

    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("Main", nullptr,
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoTitleBar
    );

    ImGui::Columns(clients.size());

     for (auto & client : clients) {
         client->drawGame();
         ImGui::NextColumn();
     }

    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);

    for (auto & client : clients) {
        client->drawConfig();
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::Separator();
    server.draw();
    ImGui::End();
}

void MainWindow::loop() {
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Projet M1");

    // check imgui OK
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    if (!ImGui::SFML::Init(window)) {
        return;
        // TODO: Error handler
    }

    sf::Clock deltaClock;
    sf::Time delta;

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

    server.shutdown();

    ImGui::SFML::Shutdown();
    ImGui::DestroyContext();

}