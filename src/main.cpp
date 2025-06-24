#include "../include/core/Board.hpp"
#include "../include/gui/Renderer.hpp"
#include "../include/engine/Search.hpp"
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    Logger::log("Starting main");
    Board board;
    Renderer renderer;
    auto search = std::make_unique<MinimaxSearch>();
    Logger::log("Renderer and search initialized");

    while (!renderer.shouldClose()) {
        Logger::log("Entering render loop");
        renderer.renderBoard(board);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                renderer.handleClick(x, y, board);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Limite à 60 FPS
        int error = 0;
        if (error != 0) std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        Logger::log("Loop iteration complete");
    }
    Logger::log("Exiting main");
    return 0;
}