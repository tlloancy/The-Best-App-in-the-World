#include "../../include/gui/Renderer.hpp"
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <sstream>

static SDL_HitTestResult SDLCALL hitTest(SDL_Window* window, const SDL_Point* pt, void* data) {
    Renderer* renderer = static_cast<Renderer*>(data);
    // Définir les limites du plateau (50-750, 100-600)
    if (pt->x < 50 || pt->x > 750 || pt->y < 100 || pt->y > 600) {
        if (renderer) renderer->logDebug("HIT-TEST: DRAGGABLE at " + std::to_string(pt->x) + ", " + std::to_string(pt->y));
        return SDL_HITTEST_DRAGGABLE; // Déplacement en dehors du plateau
    }
    if (renderer) renderer->logDebug("HIT-TEST: NORMAL at " + std::to_string(pt->x) + ", " + std::to_string(pt->y));
    return SDL_HITTEST_NORMAL; // Laisser les clics pour les pions
}

Renderer::Renderer() {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_InitSubSystem failed: " + std::string(SDL_GetError()));
    }
    window_ = SDL_CreateWindow("Cosmic Chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SDL_WINDOW_RESIZABLE);
    if (!window_) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
    }
    SDL_SetWindowSize(window_, 800, 600); // Taille explicite
    SDL_SetWindowBordered(window_, true);
    Uint32 flags = SDL_GetWindowFlags(window_);
    logDebug("Window flags: " + std::to_string(flags));
    SDL_SetWindowHitTest(window_, hitTest, this); // Passer l'instance Renderer
    renderer_ = SDL_CreateRenderer(window_, NULL);
    if (!renderer_) {
        SDL_DestroyWindow(window_);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
    }
    logDebug("Renderer created successfully");
    if (TTF_Init() < 0) {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("TTF_Init failed: " + std::string(SDL_GetError()));
    }
    font_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
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
    lastUpdate_ = std::chrono::steady_clock::now();
    lastMoveTime_ = std::chrono::steady_clock::now();
    debugEnabled_ = false; // Logs désactivés par défaut
    std::cout << "SDL3 and TTF initialized successfully" << std::endl; // Remplacement temporaire de Logger
}

Renderer::~Renderer() {
    if (font_) TTF_CloseFont(font_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    if (cursorOpen_) SDL_DestroyCursor(cursorOpen_);
    if (cursorClosed_) SDL_DestroyCursor(cursorClosed_);
    TTF_Quit();
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    std::cout << "SDL3 and TTF cleaned up" << std::endl; // Remplacement temporaire de Logger
}

void Renderer::renderBoard(const Board& board, bool isWhiteTurn) {
    SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255); // Fond sombre
    SDL_RenderClear(renderer_);

    // Plateau avec bordures subtiles
    for (int i = 0; i < 8; ++i) for (int j = 0; j < 8; ++j) {
        SDL_SetRenderDrawColor(renderer_, (i + j + 1) % 2 ? 200 : 150, (i + j + 1) % 2 ? 180 : 120, (i + j + 1) % 2 ? 160 : 100, 255);
        SDL_FRect rect = {50.0f + i * 87.5f, 100.0f + j * 62.5f, 87.0f, 62.0f};
        SDL_RenderFillRect(renderer_, &rect);
    }
    SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
    SDL_FRect borderRect = {50.0f, 100.0f, 700.0f, 500.0f};
    SDL_RenderRect(renderer_, &borderRect);

    // Pièces
    for (int idx = 0; idx < 64; ++idx) {
        if (board.getPieces()[idx]) {
            int file = idx % 8;
            int rank = 7 - (idx / 8);
            renderPiece(board.getPieces()[idx]->getType(), board.getPieces()[idx]->getColor(),
                       static_cast<int>(50 + file * 87.5), static_cast<int>(100 + rank * 62.5));
        }
    }

    // Sélection subtile
    if (getSelectedSquare() != -1) {
        int file = getSelectedSquare() % 8;
        int rank = 7 - (getSelectedSquare() / 8);
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 50);
        SDL_FRect rect = {50.0f + file * 87.5f, 100.0f + rank * 62.5f, 87.0f, 62.0f};
        SDL_RenderFillRect(renderer_, &rect);
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 100);
        SDL_FRect border = {50.0f + file * 87.5f + 1.0f, 100.0f + rank * 62.5f + 1.0f, 85.0f, 60.0f};
        SDL_RenderRect(renderer_, &border);
    }

    renderTurnIndicator(isWhiteTurn);
    renderEvaluation(0.0f);

    // Afficher les clocks avec décompte continu
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - lastUpdate_;
    if (elapsed.count() >= 1.0f) {
        if (isWhiteTurn && whiteTime_ > 0) whiteTime_ -= 1;
        else if (blackTime_ > 0) blackTime_ -= 1;
        lastUpdate_ = now;
    }
    if (whiteTime_ < 0) whiteTime_ = 0;
    if (blackTime_ < 0) blackTime_ = 0;

    std::stringstream whiteTimeStr, blackTimeStr;
    int whiteMin = whiteTime_ / 60;
    int whiteSec = whiteTime_ % 60;
    int blackMin = blackTime_ / 60;
    int blackSec = blackTime_ % 60;
    whiteTimeStr << whiteMin << ":" << (whiteSec < 10 ? "0" : "") << whiteSec;
    blackTimeStr << blackMin << ":" << (blackSec < 10 ? "0" : "") << blackSec;

    SDL_Color textColor = {220, 220, 220, 255};
    SDL_Surface* whiteSurface = TTF_RenderText_Solid(font_, whiteTimeStr.str().c_str(), whiteTimeStr.str().length(), textColor);
    SDL_Surface* blackSurface = TTF_RenderText_Solid(font_, blackTimeStr.str().c_str(), blackTimeStr.str().length(), textColor);
    if (whiteSurface && blackSurface) {
        SDL_Texture* whiteTexture = SDL_CreateTextureFromSurface(renderer_, whiteSurface);
        SDL_Texture* blackTexture = SDL_CreateTextureFromSurface(renderer_, blackSurface);
        SDL_FRect whiteRect = {20.0f, 20.0f, static_cast<float>(whiteSurface->w), static_cast<float>(whiteSurface->h)};
        SDL_FRect blackRect = {760.0f - blackSurface->w, 20.0f, static_cast<float>(blackSurface->w), static_cast<float>(blackSurface->h)};
        SDL_RenderTexture(renderer_, whiteTexture, NULL, &whiteRect);
        SDL_RenderTexture(renderer_, blackTexture, NULL, &blackRect);
        SDL_DestroyTexture(whiteTexture);
        SDL_DestroyTexture(blackTexture);
    }
    if (whiteSurface) SDL_DestroySurface(whiteSurface);
    if (blackSurface) SDL_DestroySurface(blackSurface);

    // Afficher "Time's up!" si temps écoulé
    if (whiteTime_ == 0 || blackTime_ == 0) {
        SDL_Color redColor = {255, 0, 0, 200};
        std::string timeUp = (whiteTime_ == 0 ? "Black wins! Time's up!" : "White wins! Time's up!");
        SDL_Surface* timeUpSurface = TTF_RenderText_Solid(font_, timeUp.c_str(), timeUp.length(), redColor);
        if (timeUpSurface) {
            SDL_Texture* timeUpTexture = SDL_CreateTextureFromSurface(renderer_, timeUpSurface);
            SDL_FRect timeUpRect = {400.0f - timeUpSurface->w / 2, 300.0f - timeUpSurface->h / 2, static_cast<float>(timeUpSurface->w), static_cast<float>(timeUpSurface->h)};
            SDL_RenderTexture(renderer_, timeUpTexture, NULL, &timeUpRect);
            SDL_DestroyTexture(timeUpTexture);
            SDL_DestroySurface(timeUpSurface);
        }
    }

    logDebug("Before SDL_RenderPresent");
    SDL_RenderPresent(renderer_);
    SDL_UpdateWindowSurface(window_); // Forcer l'affichage
    logDebug("After SDL_RenderPresent");
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

void Renderer::renderEvaluation(float eval) {
    SDL_SetRenderDrawColor(renderer_, eval > 0 ? 50 : 200, eval > 0 ? 200 : 50, 100, 100);
    SDL_FRect bar = {50.0f, 70.0f, 700.0f * std::abs(eval) / 10.0f, 20.0f};
    SDL_RenderFillRect(renderer_, &bar);
}

void Renderer::renderTurnIndicator(bool isWhiteTurn) {
    SDL_SetRenderDrawColor(renderer_, isWhiteTurn ? 144 : 220, isWhiteTurn ? 238 : 128, 144, 32);
    SDL_FRect indicator = {isWhiteTurn ? 10.0f : 740.0f, 100.0f, 5.0f, 500.0f};
    SDL_RenderFillRect(renderer_, &indicator);
}

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

void Renderer::handleEvents(const SDL_Event& event, Board& board, bool& isWhiteTurn) {
    int x = (event.type == SDL_EVENT_MOUSE_MOTION) ? event.motion.x : event.button.x;
    int y = (event.type == SDL_EVENT_MOUSE_MOTION) ? event.motion.y : event.button.y;
    Uint32 eventType = event.type;
    Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
    int square = getSquareFromCoords(x, y);

    if (debugEnabled_) logDebug("Event handled, type: " + std::to_string(eventType) + ", x: " + std::to_string(x) + ", y: " + std::to_string(y));

    if (eventType == SDL_EVENT_MOUSE_BUTTON_DOWN && (mouseState & SDL_BUTTON_LMASK)) {
        if (square != -1 && board.getPieces()[square] && board.getPieces()[square]->getColor() == (isWhiteTurn ? Color::White : Color::Black)) {
            setSelectedSquare(square);
            if (debugEnabled_) logDebug("Selected square: " + std::to_string(square));
            lastMoveTime_ = std::chrono::steady_clock::now();
        }
    } else if (eventType == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (getSelectedSquare() != -1 && square != -1 && square != getSelectedSquare()) {
            if (board.getPieces()[getSelectedSquare()] && board.getPieces()[getSelectedSquare()]->getColor() == (isWhiteTurn ? Color::White : Color::Black)) {
                Bitboard validMoves = board.getPieces()[getSelectedSquare()]->generateMoves(board.getOccupied(), getSelectedSquare());
                if (validMoves & Bitboard(1ULL << square) && !board.getPieces()[square]) {
                    board.movePiece(getSelectedSquare(), square);
                    if (debugEnabled_) logDebug("Move from " + std::to_string(getSelectedSquare()) + " to " + std::to_string(square));
                    isWhiteTurn = !isWhiteTurn;
                    lastMoveTime_ = std::chrono::steady_clock::now();
                } else {
                    if (debugEnabled_) logDebug("Invalid move to " + std::to_string(square));
                }
            }
            setSelectedSquare(-1);
        }
    } else if (eventType == SDL_EVENT_QUIT || (eventType == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q))) {
        closeWindow();
    } else if (eventType == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_D) {
        debugEnabled_ = !debugEnabled_;
        if (debugEnabled_) logDebug("Debug toggled to: true");
        else std::cout << "Debug toggled to: false" << std::endl; // Remplacement temporaire
    }
}

int Renderer::getSquareFromCoords(int x, int y) {
    if (x >= 50 && x <= 750 && y >= 100 && y <= 600) {
        int file = (x - 50) / 87.5;
        int rank = (y - 100) / 62.5;
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
    if (debugEnabled_) std::cout << message << std::endl; // Remplacement temporaire de Logger
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
