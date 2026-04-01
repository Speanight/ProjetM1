#include "MainWindow.hpp"

MainWindow::MainWindow(sf::Clock clock, bool quickLaunch) : console(), server(console, clock) {
    if (quickLaunch) {
        quickSetup();
    }


    thread = std::thread(&MainWindow::loop, this);
}

//MainWindow::MainWindow(sf::Clock clock, int amtPlayers) : server(clock, amtPlayers) {
//    thread = std::thread(&MainWindow::loop, this);
//}

MainWindow::~MainWindow() {
    if (thread.joinable()) {
        thread.join();
    }

    server.shutdown();
}

void MainWindow::addClient(ClientUI* client) {
    clients.push_back(client);
    client->init();
}

void MainWindow::drawGame() {
    // SIZE CONFIGURATION
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float gap = avail.y * 0.01f;
    float server_h = std::max(300.0f, avail.y * 0.3f);
    float game_h = avail.y - server_h - gap;

    // GAME AREA
    {
        ImGui::BeginChild("GameArea", ImVec2(0, game_h), false);
        // Separation between players
        ImVec2 start = ImGui::GetWindowPos();
        ImVec2 size  = ImGui::GetWindowSize();
        ImVec2 end   = ImVec2(start.x + size.x, start.y + size.y);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        float totalWidth = size.x;
        float colWidth = totalWidth / clients.size();

        for (int i = 1; i < clients.size(); i++) {
            float x = start.x + colWidth * i;

            draw_list->AddLine(
                ImVec2(x, start.y),
                ImVec2(x, end.y),
                IM_COL32(255, 255, 255, 80),
                1.0f
            );
        }

        // adding game and config
        ImGui::Columns(clients.size(), nullptr, false);

        for (auto & client : clients) {
            client->drawConfig();
            client->drawGame();
            ImGui::NextColumn();
        }

        ImGui::Columns(clients.size(), nullptr, false);
        ImGui::Columns(1);
        ImGui::EndChild();
    }

    // SERVER
    {
        ImGui::BeginChild("ServerArea", ImVec2(0, server_h), true);
        server.draw();
        ImGui::EndChild();
    }
    ImGui::End();
}

/**
 * First screen that displays when opening the game, except if quick setup is set to true
 * on MainWindow's initialisation.
 *
 * @details Allows the user to setup characters then play, or to click a quick "demo" button
 * that creates all necessary assets for demonstration purposes.
 *
 * @brief Title screen of the game. First window showing by default.
 */
void MainWindow::drawTitlescreen() {
    ImGui::Text("Projet M1");
    ImGui::Separator();

    ImGui::Columns(2);
    if (ImGui::Button("Setup & Play")) {
        screen = Screens::GAME;
    }
    ImGui::NextColumn();

    if (ImGui::Button("Demo (quick launch)")) {
        quickSetup();
        screen = Screens::GAME;
    }

    ImGui::Columns(1);
    ImGui::Separator();

    ImGui::End();
}

void MainWindow::drawPlayerSelect() {
//    int players = 2;
//
//    ImGui::SliderInt("How many players will play?", &players, 0, Const::AMT_PLAYERS_MAX);
//    players = std::max(std::min(0, players), Const::AMT_PLAYERS_MAX);
//
//    ImGui::Columns(players);
//    for (int i = 0; i <= players; i++) {
//         // TODO: Selection personnages, manettes / claviers, ...
//    }
}

void MainWindow::loop() {
    sf::RenderWindow window(sf::VideoMode({1280, 880}), "Projet M1");
    window.setFramerateLimit(60);
    window.setPosition({0,0});

    if (!ImGui::SFML::Init(window)) {
        return;
        // TODO: Error handler
    }

    ImPlot::CreateContext();

    sf::Clock deltaClock;
    sf::Time delta;

    ImGuiIO& io = ImGui::GetIO();

    // thread of window
    while (window.isOpen()) {
        delta = deltaClock.restart();
        // Closing window
        while (auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        io.DeltaTime = deltaClock.restart().asSeconds();

        auto size = window.getSize();
        io.DisplaySize = ImVec2((float)size.x, (float)size.y);

        auto mouse = sf::Mouse::getPosition(window);
        io.MousePos = ImVec2((float)mouse.x, (float)mouse.y);
        io.MouseDown[0] = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

        ImGui::SFML::Update(window, delta);

        window.clear();

        ImGui::SetNextWindowPos(ImVec2(0, 0));

        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin("Main", nullptr,
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoTitleBar);

        draw(screen);

        glClearColor(0.15f, 0.15f, 0.15f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        window.setView(window.getDefaultView());

        ImGui::SFML::Render(window);
        window.display();
    }


    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
//    ImGui::SetCurrentContext(ImGui::GetCurrentContext());
//    ImGui::DestroyContext();
}

void MainWindow::draw(short screen) {
    if (screen == Screens::TITLE_SCREEN) {drawTitlescreen();}
    if (screen == Screens::PLAYER_SELECT) {drawPlayerSelect();}
    if (screen == Screens::GAME) {drawGame();}
}

/**
 * Runs a quick setup to create 2 players with name 'Client A' and 'Client B', as well as red and green
 * colors (respectively).
 *
 * @details This function is called when MainWindow is created with a bool at true, or when clicking the
 * corresponding button in the title screen. The idea behind this function was to give a quick way to just
 * start the game and not lose time setting up players. This is particularly useful for demonstrations purposes.
 *
 * @brief Starts the game quickly by creating 2 players. Controllers are detected if available.
 */
void MainWindow::quickSetup() {
    ClientUI* clientA = new ClientUI(clock, console, "Client A", -1, sf::Color::Red);
    ClientUI* clientB = new ClientUI(clock, console, "Client B", -1, sf::Color::Green);

    sf::Joystick::update();

    if (sf::Joystick::isConnected(0)) {
        std::cout << "Controller #0 found! Associating it to " << clientA->getName();
        clientA->setController(0);
        clientA->setKeybinds({
             {Inputs::MOVEMENT_DOWN, sf::Joystick::Axis::Y},
             {Inputs::MOVEMENT_RIGHT, sf::Joystick::Axis::X},
             {Inputs::WPN_CCW, 4}, // LB button
             {Inputs::WPN_CW, 5}, // RB button
             {Inputs::WPN_CHANGE, 2}, // X (xbox HyperX)
             {Inputs::ATTACK, sf::Joystick::Axis::R}, // RT button
             {Inputs::WPN_ANGLE_WE, sf::Joystick::Axis::U},
             {Inputs::WPN_ANGLE_NS, sf::Joystick::Axis::V}
         });
    }
    else { // Fallback to keyboard keybinds.
        clientA->setKeybinds({
            {Inputs::MOVEMENT_UP, sf::Keyboard::Key::Z},
            {Inputs::MOVEMENT_DOWN, sf::Keyboard::Key::S},
            {Inputs::MOVEMENT_LEFT, sf::Keyboard::Key::Q},
            {Inputs::MOVEMENT_RIGHT, sf::Keyboard::Key::D},
            {Inputs::WPN_CCW, sf::Keyboard::Key::A},
            {Inputs::WPN_CW, sf::Keyboard::Key::E},
            {Inputs::WPN_CHANGE, sf::Keyboard::Key::W},
            {Inputs::ATTACK, sf::Keyboard::Key::C}
        });
    }

    if (sf::Joystick::isConnected(1)) {
        std::cout << "Controller #1 found! Associating it to " << clientB->getName();
        clientB->setController(1);
        clientB->setKeybinds({
             {Inputs::MOVEMENT_DOWN, sf::Joystick::Axis::Y},
             {Inputs::MOVEMENT_RIGHT, sf::Joystick::Axis::X},
             {Inputs::WPN_CCW, 4}, // LB button
             {Inputs::WPN_CW, 5}, // RB button
             {Inputs::WPN_CHANGE, 2}, // X (logitech)
             {Inputs::ATTACK, sf::Joystick::Axis::R}, // RT button
             {Inputs::WPN_ANGLE_WE, sf::Joystick::Axis::U},
             {Inputs::WPN_ANGLE_NS, sf::Joystick::Axis::V}
         });
    }
    else { // Fallback to keyboard keybinds.
        clientB->setKeybinds({
             {Inputs::MOVEMENT_UP, sf::Keyboard::Key::Up},
             {Inputs::MOVEMENT_DOWN, sf::Keyboard::Key::Down},
             {Inputs::MOVEMENT_LEFT, sf::Keyboard::Key::Left},
             {Inputs::MOVEMENT_RIGHT, sf::Keyboard::Key::Right},
             {Inputs::WPN_CCW, sf::Keyboard::Key::P},
             {Inputs::WPN_CW, sf::Keyboard::Key::M},
             {Inputs::WPN_CHANGE, sf::Keyboard::Key::L},
             {Inputs::ATTACK, sf::Keyboard::Key::O}
         });
    }

    addClient(clientA);
    addClient(clientB);

    screen = Screens::GAME;
}