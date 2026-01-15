#include <iostream>
#include "communication/Server.hpp"
#include "communication/Client.hpp"
#include "game/Position.hpp"
#include <chrono>
#include <thread>


int main() {
    // Clock will be used to sync clients, server, and refresh times (packets travels)
    auto clock = std::chrono::steady_clock::now();

    std::cout << "Starting server on IP: " << SERVER_IP << ":" << COMM_PORT_SERVER << std::endl;
    Server server(clock);
    Client client(clock, "Client A");

    std::cout << "Adding client to server..." << std::endl;
    server.addClient(client.init());

    // INFINITE LOOP - NEEDED FOR THREADS TO FUNCTION PROPERLY!
    // TODO: Threads should be blocking (eventually?)
    while (true) {}

    server.shutdown();
    return 0;
}