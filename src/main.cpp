#include <SDL3/SDL.h>
#include <iostream>
#include "gui/Renderer.hpp"
#include "core/Board.hpp"

Renderer* globalRenderer = nullptr;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    Board board;
    bool isWhiteTurn = true;
    Renderer renderer;
    globalRenderer = &renderer;

    bool running = true;
    while (running) {
        if (!renderer.shouldClose()) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                renderer.handleEvents(event, board, isWhiteTurn);
            }
            renderer.renderBoard(board, isWhiteTurn);
        } else {
            running = false;
        }
    }

    SDL_Quit();
    return 0;
}