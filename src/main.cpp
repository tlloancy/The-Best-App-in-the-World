#include "gui/Renderer.hpp"
#include "core/Board.hpp"
#include <SDL3/SDL.h>

int main(int argc, char* argv[]) {
    bool debugEnabled = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--debug") {
            debugEnabled = true;
        }
    }
    Board board;
    bool isWhiteTurn = true;
    Renderer renderer(1000, 800, debugEnabled, true);
    while (!renderer.shouldClose()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            renderer.handleEvents(event, board, isWhiteTurn);
        }
        renderer.renderBoard(board, isWhiteTurn);
        SDL_Delay(10);
    }
    return 0;
}
