#include "../../include/gui/Renderer.hpp"
#include <stdexcept>
#include <iostream>
#include <chrono>

Renderer::Renderer() {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_InitSubSystem failed: " + std::string(SDL_GetError()));
    }
    window_ = SDL_CreateWindow("Cosmic Chess", 800, 600, 0);
    if (!window_) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
    }
    renderer_ = SDL_CreateRenderer(window_, NULL);
    if (!renderer_) {
        SDL_DestroyWindow(window_);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
    }
    if (TTF_Init() < 0) {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("TTF_Init failed: " + std::string(SDL_GetError()));
    }
    font_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 40);
    if (!font_) {
        TTF_Quit();
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("TTF_OpenFont failed: " + std::string(SDL_GetError()));
    }
    cursorOpen_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    cursorClosed_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
    SDL_SetCursor(cursorOpen_);
    Logger::log("SDL3 and TTF initialized successfully");
}

Renderer::~Renderer() {
    if (font_) TTF_CloseFont(font_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    if (cursorOpen_) SDL_DestroyCursor(cursorOpen_);
    if (cursorClosed_) SDL_DestroyCursor(cursorClosed_);
    TTF_Quit();
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    Logger::log("SDL3 and TTF cleaned up");
}

void Renderer::renderBoard(const Board& board) {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    for (int i = 0; i < 8; ++i) for (int j = 0; j < 8; ++j) {
        SDL_SetRenderDrawColor(renderer_, (i + j + 1) % 2 ? 221 : 166, (i + j + 1) % 2 ? 184 : 109, (i + j + 1) % 2 ? 140 : 79, 255);
        SDL_FRect rect = {static_cast<float>(50 + (7 - i) * 87.5), static_cast<float>(50 + (7 - j) * 62.5), 87.0f, 62.0f};
        SDL_RenderFillRect(renderer_, &rect);
    }
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    for (int i = 0; i <= 8; ++i) {
        SDL_RenderLine(renderer_, 50.0f + i * 87.5f, 50.0f, 50.0f + i * 87.5f, 550.0f);
        SDL_RenderLine(renderer_, 50.0f, 50.0f + i * 62.5f, 750.0f, 50.0f + i * 62.5f);
    }
    for (int idx = 0; idx < 64; ++idx) {
        if (board.getPieces()[idx]) {
            int file = idx % 8;
            int rank = 7 - (idx / 8);
            renderPiece(board.getPieces()[idx]->getType(), board.getPieces()[idx]->getColor(),
                       static_cast<int>(50 + file * 87.5), static_cast<int>(50 + rank * 62.5));
        }
    }
    if (selectedSquare_ != -1) {
        int file = selectedSquare_ % 8;
        int rank = 7 - (selectedSquare_ / 8);
        SDL_SetRenderDrawColor(renderer_, 0, 128, 255, 128); // Semi-transparent blue for selection
        SDL_FRect rect = {static_cast<float>(50 + file * 87.5), static_cast<float>(50 + rank * 62.5), 87.0f, 62.0f};
        SDL_RenderFillRect(renderer_, &rect);
    }
    SDL_RenderPresent(renderer_);
}

void Renderer::renderPiece(PieceType type, Color color, int x, int y) {
    std::string symbol;
    switch (type) {
        case PieceType::Pawn: symbol = "P"; break;
        case PieceType::Knight: symbol = "N"; break;
        case PieceType::Bishop: symbol = "B"; break;
        case PieceType::Rook: symbol = "R"; break;
        case PieceType::Queen: symbol = "Q"; break;
        case PieceType::King: symbol = "K"; break;
    }
    if (color == Color::Black) symbol = std::string(1, tolower(symbol[0]));
    SDL_Color textColor = {255, 255, 255, 255};
    size_t length = symbol.length();
    SDL_Surface* surface = TTF_RenderText_Solid(font_, symbol.c_str(), length, textColor);
    if (!surface) {
        logDebug("TTF_RenderText_Solid failed: " + std::string(SDL_GetError()));
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FRect rect = {static_cast<float>(x + 20), static_cast<float>(y + 10), 50.0f, 40.0f};
    SDL_RenderTexture(renderer_, texture, NULL, &rect);
    SDL_DestroySurface(surface);
    SDL_DestroyTexture(texture);
}

void Renderer::renderSpiral(float evaluation) {}

void Renderer::swapBuffers() {}

bool Renderer::shouldClose() {
    static auto lastTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count() > 16) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) return true;
            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q)) return true;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_D) debugEnabled_ = !debugEnabled_;
        }
        lastTime = currentTime;
    }
    return false;
}

void Renderer::handleEvents(const SDL_Event& event, Board& board) {
    int x = (event.type == SDL_EVENT_MOUSE_MOTION) ? event.motion.x : event.button.x;
    int y = (event.type == SDL_EVENT_MOUSE_MOTION) ? event.motion.y : event.button.y;
    Uint32 eventType = event.type;
    Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
    int square = getSquareFromCoords(x, y);

    logDebug("Event handled, type: " + std::to_string(eventType) + ", x: " + std::to_string(x) + ", y: " + std::to_string(y));

    if (eventType == SDL_EVENT_MOUSE_BUTTON_DOWN && (mouseState & SDL_BUTTON_LMASK)) {
        if (square != -1 && board.getPieces()[square] && !isDragging_) {
            selectedSquare_ = square;
            dragStartX_ = x;
            dragStartY_ = y;
            isDragging_ = true;
            logDebug("Drag started on square: " + std::to_string(square));
            updateCursor(true);
        } else if (square != -1 && board.getPieces()[square] && !isClickSelecting_) {
            selectedSquare_ = square;
            isClickSelecting_ = true;
            logDebug("Click selected square: " + std::to_string(square));
        }
    } else if (eventType == SDL_EVENT_MOUSE_MOTION && (mouseState & SDL_BUTTON_LMASK) && isDragging_) {
        logDebug("Dragging to x: " + std::to_string(x) + ", y: " + std::to_string(y));
    } else if (eventType == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (isDragging_ && square != -1 && square != selectedSquare_) {
            if (board.getPieces()[selectedSquare_]) {
                Bitboard validMoves = board.getPieces()[selectedSquare_]->generateMoves(board.getOccupied(), selectedSquare_);
                if (validMoves & Bitboard(1ULL << square) && !board.getPieces()[square]) {
                    board.movePiece(selectedSquare_, square);
                    logDebug("Drag move from " + std::to_string(selectedSquare_) + " to " + std::to_string(square));
                } else {
                    logDebug("Invalid drag move to " + std::to_string(square));
                }
            }
            selectedSquare_ = -1;
            isDragging_ = false;
            updateCursor(false);
        } else if (isClickSelecting_ && square != -1 && square != selectedSquare_) {
            if (board.getPieces()[selectedSquare_]) {
                Bitboard validMoves = board.getPieces()[selectedSquare_]->generateMoves(board.getOccupied(), selectedSquare_);
                if (validMoves & Bitboard(1ULL << square) && !board.getPieces()[square]) {
                    board.movePiece(selectedSquare_, square);
                    logDebug("Click move from " + std::to_string(selectedSquare_) + " to " + std::to_string(square));
                } else {
                    logDebug("Invalid click move to " + std::to_string(square));
                }
            }
            selectedSquare_ = -1;
            isClickSelecting_ = false;
        }
    }
}

int Renderer::getSquareFromCoords(int x, int y) {
    if (x >= 50 && x <= 750 && y >= 50 && y <= 550) {
        int file = (x - 50) / 87.5;
        int rank = (y - 50) / 62.5;
        if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
            return (7 - rank) * 8 + file;
        }
    }
    return -1;
}

void Renderer::updateCursor(bool isDragging) {
    SDL_SetCursor(isDragging ? cursorClosed_ : cursorOpen_);
}

void Renderer::logDebug(const std::string& message) {
    if (debugEnabled_) Logger::log(message);
}

bool Renderer::isKeyPressed(int key) const {
    const Uint8* state = (const Uint8*)SDL_GetKeyboardState(NULL);
    SDL_Keymod modstate = SDL_GetModState();
    return state[SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key), &modstate)];
}

void Renderer::closeWindow() {
    SDL_Event quitEvent;
    quitEvent.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quitEvent);
}
