#include "MainWindow.hpp"

MainWindow::MainWindow(sf::Clock clock, bool quickLaunch) : console(), server(console, clock) {
    if (quickLaunch) {
        gameSetup();
    }
    else {
        screen = Screens::TITLE_SCREEN;
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
        int count = clients.size();

        if (count <= 4) {
            ImGui::Columns(count, nullptr, false);

            for (auto& client : clients) {
                client->drawConfig();
                client->drawGame();
                ImGui::NextColumn();
            }
        }
        else {
            int topCount = (count + 1) / 2;
            int bottomCount = count - topCount;

            ImGui::Columns(topCount, nullptr, false);

            for (int i = 0; i < topCount; i++) {
                clients[i]->drawConfig();
                clients[i]->drawGame();
                ImGui::NextColumn();
            }

            ImGui::Columns(1);
            ImGui::Separator();

            ImGui::Columns(bottomCount, nullptr, false);

            for (int i = topCount; i < count; i++) {
                clients[i]->drawConfig();
                clients[i]->drawGame();
                ImGui::NextColumn();
            }
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
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float maxWidth = avail.x * 0.8f;

    static int nbPlayers = 2;
    static bool modeNormal = true;      // default mode selected = normal
    static bool modeDemo = false;

    // Zone split 20 / 60 / 20
    float titleHeight = avail.y * 0.2f;
    float menuHeight  = avail.y * 0.6f;
    float bottomHeight = avail.y * 0.2f;

    // Title
    {
        ImGui::BeginChild("TitleZone", ImVec2(0, titleHeight), false);

        float fontSize = maxWidth * 0.08f;
        if (fontSize < 100.0f) fontSize = 100.0f;

        ImGui::SetWindowFontScale(fontSize / ImGui::GetFontSize());

        const char* title = "Choisissez mode de jeu";
        ImVec2 textSize = ImGui::CalcTextSize(title);

        ImGui::SetCursorPosX((avail.x - textSize.x) * 0.5f);
        ImGui::SetCursorPosY((titleHeight - textSize.y) * 0.5f);

        ImGui::Text("%s", title);

        ImGui::SetWindowFontScale(1.0f);

        ImGui::EndChild();
    }

    // Selection menu
    {
        ImGui::BeginChild("MenuZone", ImVec2(0, menuHeight), false);

        float textScale = maxWidth * 0.02f;
        if (textScale < 20.0f) textScale = 20.0f;

        ImGui::SetWindowFontScale(textScale / ImGui::GetFontSize());

        ImGui::Columns(2, nullptr, false);

        float columnWidth = ImGui::GetColumnWidth();
        float innerPadding = columnWidth * 0.1f;
        float textPadding = 15.0f;

        // Demo
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + innerPadding);

            const char* label = "Mode démo";
            ImVec2 size = ImGui::CalcTextSize(label);

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - innerPadding*2 - size.x) * 0.5f);
            ImGui::Checkbox(label, &modeDemo);
            if (modeDemo) modeNormal = false;

            ImGui::Dummy(ImVec2(0, 20));

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + innerPadding + textPadding);

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + columnWidth - innerPadding*2 - textPadding);
            ImGui::TextWrapped("Deux joueurs commencent avec 0 points au départ, leur objectif ? en accumuler le plus en attaquant leur adversaire. Ce mode de jeu est sans fin et est idéal pour la démonstration du jeu.");
            ImGui::PopTextWrapPos();
        }

        ImGui::NextColumn();

        // Normal
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + innerPadding);

            const char* label = "Mode normal";
            ImVec2 size = ImGui::CalcTextSize(label);

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - innerPadding*2 - size.x) * 0.5f);
            ImGui::Checkbox(label, &modeNormal);
            if (modeNormal) modeDemo = false;

            ImGui::Dummy(ImVec2(0, 20));

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + innerPadding + textPadding);

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + columnWidth - innerPadding*2 - textPadding);
            ImGui::TextWrapped("Chaque joueurs commencent avec 100 point de vie au départ, leur objectif ? tuer les autres joueurs. Ce mode de jeu est fait pour imiter un jeu dans son ensemble de l'attribution des joueurs a la gestion de la fin de partie.");
            ImGui::PopTextWrapPos();

            ImGui::Dummy(ImVec2(0, 30)); // get the more space he can

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + innerPadding);

            ImGui::Text("Nombre de joueurs :");

            // largeur dynamique avec minimum
            float availableWidth = columnWidth - innerPadding * 2;
            float inputWidth = availableWidth * 0.4f;

            if (inputWidth < 120.0f) inputWidth = 120.0f; // cancell disapiration
            if (inputWidth > 200.0f) inputWidth = 200.0f; // make it not too large

            // centrage
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availableWidth - inputWidth) * 0.5f);

            ImGui::PushItemWidth(inputWidth);
            if (ImGui::InputInt("##players", &nbPlayers, 1, 1)) {
                if (nbPlayers < 2) nbPlayers = 2;
                if (nbPlayers > 4) nbPlayers = 4;
            }
            ImGui::PopItemWidth();
        }

        ImGui::Columns(1);
        ImGui::SetWindowFontScale(1.0f);

        ImGui::EndChild();
    }

    // Buttons
    {
        ImGui::BeginChild("BottomZone", ImVec2(0, bottomHeight), false);

        float buttonWidth = ImGui::GetContentRegionAvail().x * 0.4f;
        float buttonHeight = 50.0f;

        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - buttonWidth) * 0.5f);
        ImGui::SetCursorPosY((bottomHeight - buttonHeight) * 0.5f);

        if (ImGui::Button("Lancer le jeu", ImVec2(buttonWidth, buttonHeight))) {
            if (nbPlayers < 2) nbPlayers = 2;
            if (nbPlayers > 4) nbPlayers = 4;

            if (modeNormal) {
                gameSetup(nbPlayers);
            }
            else if (modeDemo) {
                demoSetup();
            }
            else {
                gameSetup(2);
            }
        }

        ImGui::EndChild();
    }

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
void MainWindow::demoSetup() {
    ClientUI* clientA = new ClientUI(clock, console, "Client A", -1, sf::Color::Red);
    ClientUI* clientB = new ClientUI(clock, console, "Client B", -1, sf::Color::Green);

    sf::Joystick::update();

    if (sf::Joystick::isConnected(0)) {
        std::cout << "Controller #0 found! Associating it to " << clientA->getName();
        clientA->setController(0);
        clientA->setKeybinds(Controller::CONTROLLER_MAP[0]);
    }
    else { // Fallback to keyboard keybinds.
        clientA->setKeybinds(Controller::KEYBIND_MAP[0]);
    }

    if (sf::Joystick::isConnected(1)) {
        std::cout << "Controller #1 found! Associating it to " << clientB->getName();
        clientB->setController(1);
        clientB->setKeybinds(Controller::CONTROLLER_MAP[1]);
    }
    else { // Fallback to keyboard keybinds.
        clientB->setKeybinds(Controller::KEYBIND_MAP[1]);
    }

    addClient(clientA);
    addClient(clientB);

    this->server.setMaxPlayers(2);
    this->server.setDemoMode(true);

    screen = Screens::GAME;
}

void MainWindow::gameSetup(int nbPlayers) {
    int controllerAvailable = 0;
    int keyboardAvailable = 0;

    sf::Joystick::update();
    for(int i = 0; i < nbPlayers; i++) {
        ClientUI* client = new ClientUI(clock, console, "Client"+std::to_string(i), -1, sf::Color::Red);

        if (sf::Joystick::isConnected(0)||sf::Joystick::isConnected(1)||sf::Joystick::isConnected(2)) {
            std::cout<<"attributing controller "<<controllerAvailable<<"..."<<std::endl;
            client->setKeybinds(Controller::CONTROLLER_MAP[controllerAvailable]);
            client->setController(controllerAvailable);
            controllerAvailable++;
        }
        else {
            std::cout<<"attributing keyboard "<<keyboardAvailable<<"..."<<std::endl;
            client->setKeybinds(Controller::KEYBIND_MAP[keyboardAvailable]);
            keyboardAvailable++;
        }

        client->setLoop(true);
        addClient(client);
    }
    this->server.setMaxPlayers(nbPlayers);
    this->server.setDemoMode(false);
    screen = Screens::GAME;
}