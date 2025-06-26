#include "../include/core/Board.hpp"
#include "../include/gui/Renderer.hpp"
#include "../include/engine/Search.hpp"
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include "../include/utils/Logger.hpp"

int main() {
    Logger::log("Starting application");
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        Logger::log("SDL_InitSubSystem failed: " + std::string(SDL_GetError()));
        return 1;
    }
    Board board;
    Renderer renderer;
    auto search = std::make_unique<MinimaxSearch>();
    Logger::log("Renderer and search initialized");
    bool isWhiteTurn = true;
    bool running = true;

    while (running) {
        renderer.renderBoard(board, isWhiteTurn);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            renderer.handleEvents(event, board, isWhiteTurn);
            if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q))) {
                running = false;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        int error = 0;
        if (error != 0) std::cerr << "SDL Error: " + std::string(SDL_GetError()) << std::endl;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    Logger::log("Exiting application");
    return 0;
}