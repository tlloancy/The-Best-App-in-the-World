#include "../../include/gui/Renderer.hpp"
#include <stdexcept>
#include <iostream>
#include <chrono>

Renderer::Renderer() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
    }
    window_ = SDL_CreateWindow("Cosmic Chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window_) {
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
    }
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) {
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
    }
    if (TTF_Init() < 0) {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error("TTF_Init failed: " + std::string(TTF_GetError()));
    }
    font_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 40);
    if (!font_) {
        TTF_Quit();
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error("TTF_OpenFont failed: " + std::string(TTF_GetError()));
    }
    // Curseurs personnalisés
    cursorOpen_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    cursorClosed_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    SDL_SetCursor(cursorOpen_);
    Logger::log("SDL and TTF initialized successfully");
}

Renderer::~Renderer() {
    if (font_) TTF_CloseFont(font_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    if (cursorOpen_) SDL_FreeCursor(cursorOpen_);
    if (cursorClosed_) SDL_FreeCursor(cursorClosed_);
    TTF_Quit();
    SDL_Quit();
    Logger::log("SDL and TTF cleaned up");
}

void Renderer::renderBoard(const Board& board) {
    logDebug("Rendering board");
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255); // Fond noir
    SDL_RenderClear(renderer_);
    for (int i = 0; i < 8; ++i) for (int j = 0; j < 8; ++j) {
        SDL_SetRenderDrawColor(renderer_, (i + j + 1) % 2 ? 221 : 166, (i + j + 1) % 2 ? 184 : 109, (i + j + 1) % 2 ? 140 : 79, 255);
        SDL_Rect rect = {static_cast<int>(50 + (7 - i) * 87.5), static_cast<int>(50 + (7 - j) * 62.5), 87, 62};
        SDL_RenderFillRect(renderer_, &rect);
    }
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255); // Blanc
    for (int i = 0; i <= 8; ++i) {
        SDL_RenderDrawLine(renderer_, static_cast<int>(50 + i * 87.5), 50, static_cast<int>(50 + i * 87.5), 550);
        SDL_RenderDrawLine(renderer_, 50, static_cast<int>(50 + i * 62.5), 750, static_cast<int>(50 + i * 62.5));
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
        SDL_SetRenderDrawColor(renderer_, 0, 128, 255, 64); // Bleu clair très transparent
        SDL_Rect rect = {static_cast<int>(50 + file * 87.5), static_cast<int>(50 + rank * 62.5), 87, 62};
        SDL_RenderFillRect(renderer_, &rect);
    }
    SDL_RenderPresent(renderer_);
    logDebug("Board with pieces rendered");
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
    SDL_Color textColor = {255, 255, 255, 255}; // Blanc
    SDL_Surface* surface = TTF_RenderText_Solid(font_, symbol.c_str(), textColor);
    if (!surface) {
        logDebug("TTF_RenderText_Solid failed: " + std::string(TTF_GetError()));
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_Rect rect = {x + 20, y + 10, 50, 40};
    SDL_RenderCopy(renderer_, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    logDebug("Rendered piece " + symbol + " at " + std::to_string(x) + ", " + std::to_string(y));
}

void Renderer::renderSpiral(float evaluation) {
    // Désactivé temporairement
}

void Renderer::swapBuffers() {
    // Pas besoin avec SDL, géré par RenderPresent
}

bool Renderer::shouldClose() {
    static auto lastTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count() > 16) { // ~60 FPS
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            logDebug("Event type: " + std::to_string(event.type));
            if (event.type == SDL_QUIT) return true;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) return true;
                if (event.key.keysym.sym == SDLK_d) debugEnabled_ = !debugEnabled_; // Toggle debug
            }
        }
        lastTime = currentTime;
    }
    return false;
}

void Renderer::handleEvents(int x, int y, Uint32 eventType, Board& board) {
    int file = (x - 50) / 87.5;
    int rank = (y - 50) / 62.5;
    if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
        int square = (7 - rank) * 8 + file;
        logDebug("Event on square: " + std::to_string(square) + ", type: " + std::to_string(eventType));
        updateCursor(eventType == SDL_MOUSEMOTION && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK));
        if (eventType == SDL_MOUSEBUTTONDOWN && board.getPieces()[square]) {
            selectedSquare_ = square;
            logDebug("Selected square: " + std::to_string(square));
        } else if (eventType == SDL_MOUSEBUTTONUP && selectedSquare_ != -1) {
            if (board.getPieces()[selectedSquare_]) {
                Bitboard validMoves = board.getPieces()[selectedSquare_]->generateMoves(board.getOccupied(), selectedSquare_);
                if (validMoves & Bitboard(1ULL << square) && !board.getPieces()[square]) {
                    if (board.movePiece(selectedSquare_, square)) {
                        logDebug("Moved from " + std::to_string(selectedSquare_) + " to " + std::to_string(square));
                    }
                } else {
                    logDebug("Invalid move to " + std::to_string(square));
                }
            }
            selectedSquare_ = -1;
        }
    } else if (eventType == SDL_MOUSEBUTTONUP) {
        selectedSquare_ = -1;
    }
}

void Renderer::updateCursor(bool isDragging) {
    SDL_SetCursor(isDragging ? cursorClosed_ : cursorOpen_);
}

void Renderer::logDebug(const std::string& message) {
    if (debugEnabled_) Logger::log(message);
}

bool Renderer::isKeyPressed(int key) const {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    return state[SDL_GetScancodeFromKey(key)];
}

void Renderer::closeWindow() {
    SDL_Event quitEvent;
    quitEvent.type = SDL_QUIT;
    SDL_PushEvent(&quitEvent);
}