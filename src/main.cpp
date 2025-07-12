#include "gui/Renderer.hpp"
#include "core/Board.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

int main(int argc, char** argv) {
    bool debugEnabled = (argc > 1 && std::string(argv[1]) == "--debug");
    {
        Renderer renderer(1000, 800, debugEnabled);
        Board board;
        bool isWhiteTurn = true;
        while (!renderer.shouldClose()) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                renderer.handleEvents(event, board, isWhiteTurn);
            }
            renderer.renderBoard(board, isWhiteTurn);
        }
    }
    TTF_Quit();
    SDL_Quit();
    return 0;
}
