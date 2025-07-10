#include "../../include/gui/Renderer.hpp"
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <sstream>
#include <cstring>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

static SDL_HitTestResult SDLCALL hitTest(SDL_Window* window, const SDL_Point* pt, void* data) {
    Renderer* renderer = static_cast<Renderer*>(data);
    if (pt->x < 50 || pt->x > 750 || pt->y < 100 || pt->y > 600) {
        return SDL_HITTEST_DRAGGABLE;
    }
    return SDL_HITTEST_NORMAL;
}

Renderer::Renderer() {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_InitSubSystem failed: " + std::string(SDL_GetError()));
    }
    if (TTF_Init() < 0) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("TTF_Init failed: " + std::string(SDL_GetError()));
    }
    window_ = SDL_CreateWindow("Cosmic Chess", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window_) {
        TTF_Quit();
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
    }
    SDL_SetWindowHitTest(window_, hitTest, this);
    renderer_ = SDL_CreateRenderer(window_, NULL);
    if (!renderer_) {
        SDL_DestroyWindow(window_);
        TTF_Quit();
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
    }
    const char *basePath = SDL_GetBasePath();
    basePath_ = basePath ? basePath : "./";
    logDebug("SDL_GetBasePath returned: " + basePath_);
    if (basePath) SDL_free((void*)basePath);
    font_ = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 24);
    if (!font_) {
        logDebug("TTF_OpenFont failed for /usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf: " + std::string(SDL_GetError()));
        font_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
        if (!font_) {
            logDebug("TTF_OpenFont failed for /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: " + std::string(SDL_GetError()));
        }
    }
    cursorOpen_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    cursorClosed_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
    SDL_SetCursor(cursorOpen_);
    lastUpdate_ = std::chrono::steady_clock::now();
    lastMoveTime_ = std::chrono::steady_clock::now();
    debugEnabled_ = true;
    lastStateChanged = false;
    quitPromptActive_ = false;
    quitConfirmed_ = false;
    lastKeyPressTime_ = 0;
    isDragging_ = false;
    dragX_ = -1;
    dragY_ = -1;
    std::cout << "SDL3 and SDL_image initialized successfully" << std::endl;
}

Renderer::~Renderer() {
    for (auto& pair : textureCache_) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
            pair.second = nullptr;
        }
    }
    textureCache_.clear();
    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    if (cursorOpen_) {
        SDL_DestroyCursor(cursorOpen_);
        cursorOpen_ = nullptr;
    }
    if (cursorClosed_) {
        SDL_DestroyCursor(cursorClosed_);
        cursorClosed_ = nullptr;
    }
    TTF_Quit();
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    if (isKeyPressed(SDLK_D)) std::cout << "SDL3 and SDL_image cleaned up" << std::endl;
}

void Renderer::renderBoard(const Board& board, bool isWhiteTurn) {
    SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
    SDL_RenderClear(renderer_);

    for (int i = 0; i < 8; ++i) for (int j = 0; j < 8; ++j) {
        SDL_SetRenderDrawColor(renderer_, (i + j + 1) % 2 ? 200 : 150, (i + j + 1) % 2 ? 180 : 120, (i + j + 1) % 2 ? 160 : 100, 255);
        SDL_FRect rect = {50.0f + i * 87.5f, 100.0f + j * 62.5f, 87.0f, 62.0f};
        SDL_RenderFillRect(renderer_, &rect);
    }
    SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
    SDL_FRect borderRect = {50.0f, 100.0f, 700.0f, 500.0f};
    SDL_RenderRect(renderer_, &borderRect);

    for (int idx = 0; idx < 64; ++idx) {
        if (board.getPieces()[idx] && idx != selectedSquare_) {
            int file = idx % 8;
            int rank = 7 - (idx / 8);
            renderPiece(board.getPieces()[idx]->getType(), board.getPieces()[idx]->getColor(),
                        static_cast<int>(50 + file * 87.5 + 18), static_cast<int>(100 + rank * 62.5 + 6));
        }
    }

    if (isDragging_ && selectedSquare_ != -1 && board.getPieces()[selectedSquare_]) {
        renderPiece(board.getPieces()[selectedSquare_]->getType(), board.getPieces()[selectedSquare_]->getColor(),
                    dragX_ - 25, dragY_ - 25);
    }

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

    if (lastStateChanged) {
        if (board.isCheckmate()) {
            std::string message = board.isWhiteToMove() ? "Black wins by Checkmate!" : "White wins by Checkmate!";
            renderGameEnd(message, {255, 0, 0, 255});
        } else if (board.isStalemate()) {
            renderGameEnd("Stalemate! Draw!", {0, 255, 0, 255});
        } else if (board.isDraw()) {
            renderGameEnd("Draw by 50-move rule!", {0, 0, 255, 255});
        } else if (board.isCheck()) {
            std::string message = "Check!";
            renderGameEnd(message, {255, 255, 0, 255});
        }
        lastStateChanged = false;
    }

    renderTurnIndicator(isWhiteTurn);
    renderEvaluation(0.0f);

    if (font_) {
        SDL_Color whiteColor = {255, 255, 255, 255};
        std::string whiteTimeStr = "White: " + std::to_string(static_cast<int>(whiteTime_)) + "s";
        std::string blackTimeStr = "Black: " + std::to_string(static_cast<int>(blackTime_)) + "s";
        SDL_Surface* whiteSurface = TTF_RenderText_Blended(font_, whiteTimeStr.c_str(), whiteTimeStr.length(), whiteColor);
        SDL_Surface* blackSurface = TTF_RenderText_Blended(font_, blackTimeStr.c_str(), blackTimeStr.length(), whiteColor);
        if (whiteSurface && blackSurface) {
            SDL_Texture* whiteTexture = SDL_CreateTextureFromSurface(renderer_, whiteSurface);
            SDL_Texture* blackTexture = SDL_CreateTextureFromSurface(renderer_, blackSurface);
            if (whiteTexture && blackTexture) {
                SDL_FRect whiteRect = {10.0f, 10.0f, static_cast<float>(whiteSurface->w), static_cast<float>(whiteSurface->h)};
                SDL_FRect blackRect = {10.0f, 40.0f, static_cast<float>(blackSurface->w), static_cast<float>(blackSurface->h)};
                SDL_RenderTexture(renderer_, whiteTexture, NULL, &whiteRect);
                SDL_RenderTexture(renderer_, blackTexture, NULL, &blackRect);
                SDL_DestroyTexture(whiteTexture);
                SDL_DestroyTexture(blackTexture);
            } else {
                logDebug("Failed to create textures for time display: " + std::string(SDL_GetError()));
            }
            SDL_DestroySurface(whiteSurface);
            SDL_DestroySurface(blackSurface);
        } else {
            logDebug("Failed to render time text: " + std::string(SDL_GetError()));
        }
    }

    if (quitPromptActive_) {
        renderGameEnd("Quit? Press Y to confirm, N to cancel", {255, 255, 255, 255}, true);
    }

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - lastUpdate_;
    if (elapsed.count() >= 1.0f) {
        if (isWhiteTurn && whiteTime_ > 0) whiteTime_ -= 1;
        else if (!isWhiteTurn && blackTime_ > 0) blackTime_ -= 1;
        lastUpdate_ = now;
    }
    if (whiteTime_ < 0) whiteTime_ = 0;
    if (blackTime_ < 0) blackTime_ = 0;

    if (isKeyPressed(SDLK_D)) logDebug("Before SDL_RenderPresent");
    SDL_RenderPresent(renderer_);
    if (isKeyPressed(SDLK_D)) logDebug("After SDL_RenderPresent");
}

void Renderer::renderPiece(PieceType type, Color color, int x, int y) {
    std::pair<PieceType, Color> key = {type, color};
    if (textureCache_.find(key) == textureCache_.end()) {
        std::string imagePath = basePath_ + "../assets/";
        std::string pieceName;
        switch (type) {
            case PieceType::Pawn: pieceName = (color == Color::White) ? "white_pawn" : "black_pawn"; break;
            case PieceType::Knight: pieceName = (color == Color::White) ? "white_knight" : "black_knight"; break;
            case PieceType::Bishop: pieceName = (color == Color::White) ? "white_bishop" : "black_bishop"; break;
            case PieceType::Rook: pieceName = (color == Color::White) ? "white_rook" : "black_rook"; break;
            case PieceType::Queen: pieceName = (color == Color::White) ? "white_queen" : "black_queen"; break;
            case PieceType::King: pieceName = (color == Color::White) ? "white_king" : "black_king"; break;
        }
        imagePath += pieceName + ".png";

        SDL_Surface* surface = IMG_Load(imagePath.c_str());
        if (!surface) {
            logDebug("IMG_Load failed for " + imagePath + ": " + std::string(SDL_GetError()));
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        if (!texture) {
            logDebug("SDL_CreateTextureFromSurface failed for " + imagePath + ": " + std::string(SDL_GetError()));
            SDL_DestroySurface(surface);
            return;
        }
        textureCache_[key] = texture;
        SDL_DestroySurface(surface);
    }

    SDL_FRect rect = {static_cast<float>(x), static_cast<float>(y), 50.0f, 50.0f};
    if (SDL_RenderTexture(renderer_, textureCache_[key], NULL, &rect) < 0) {
        logDebug("SDL_RenderTexture failed for piece at x=" + std::to_string(x) + ", y=" + std::to_string(y) + ": " + std::string(SDL_GetError()));
    }
}

void Renderer::renderGameEnd(const std::string& message, const SDL_Color& color, bool isQuitPrompt) {
    if (font_) {
        SDL_Surface* surface = TTF_RenderText_Blended(font_, message.c_str(), message.length(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
            if (texture) {
                SDL_FRect rect = {isQuitPrompt ? 100.0f : 200.0f, 300.0f, static_cast<float>(surface->w), static_cast<float>(surface->h)};
                SDL_RenderTexture(renderer_, texture, NULL, &rect);
                SDL_DestroyTexture(texture);
            } else {
                logDebug("Failed to create texture for game end message: " + std::string(SDL_GetError()));
            }
            SDL_DestroySurface(surface);
        } else {
            logDebug("Failed to render game end text: " + std::string(SDL_GetError()));
        }
    }
}

void Renderer::renderSpiral(float evaluation) {}

void Renderer::renderEvaluation(float eval) {
    if (isKeyPressed(SDLK_D)) {
        SDL_SetRenderDrawColor(renderer_, eval > 0 ? 50 : 200, eval > 0 ? 200 : 50, 100, 100);
        SDL_FRect bar = {50.0f, 70.0f, 700.0f * std::min(std::abs(eval) / 10.0f, 1.0f), 20.0f};
        SDL_RenderFillRect(renderer_, &bar);
    }
}

void Renderer::renderTurnIndicator(bool isWhiteTurn) {
    SDL_SetRenderDrawColor(renderer_, isWhiteTurn ? 144 : 220, isWhiteTurn ? 238 : 128, 144, 32);
    SDL_FRect indicator = {isWhiteTurn ? 10.0f : 740.0f, 100.0f, 5.0f, 500.0f};
    SDL_RenderFillRect(renderer_, &indicator);
}

void Renderer::swapBuffers() {}

bool Renderer::shouldClose() {
    return quitConfirmed_;
}

void Renderer::handleEvents(const SDL_Event& event, Board& board, bool& isWhiteTurn) {
    int x = -1, y = -1;
    if (debugEnabled_) {
        logDebug("Handling event type: " + std::to_string(event.type));
    }
    Uint32 currentTime = SDL_GetTicks();
    if (event.type == SDL_EVENT_MOUSE_MOTION && isDragging_) {
        x = event.motion.x;
        y = event.motion.y;
        dragX_ = x;
        dragY_ = y;
        updateCursor(true);
        if (debugEnabled_) {
            logDebug("Dragging, mouse at x=" + std::to_string(x) + ", y=" + std::to_string(y));
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
        x = event.button.x;
        y = event.button.y;
        int square = getSquareFromCoords(x, y);
        if (debugEnabled_) {
            logDebug("Mouse down, type: " + std::to_string(event.type) + ", x=" + std::to_string(x) + ", y=" + std::to_string(y) + ", square: " + std::to_string(square));
        }
        if (square != -1 && board.getPieces()[square] && board.getPieces()[square]->getColor() == (isWhiteTurn ? Color::White : Color::Black)) {
            setSelectedSquare(square);
            isDragging_ = true;
            dragX_ = x;
            dragY_ = y;
            updateCursor(true);
            if (debugEnabled_) {
                logDebug("Selected square: " + std::to_string(square) + ", piece type: " + std::to_string(static_cast<int>(board.getPieces()[square]->getType())));
                std::ostringstream oss;
                oss << std::hex << board.getPieces()[square]->generateMoves(board.getOccupied(), square).getValue() << std::dec;
                logDebug("Valid moves for " + std::to_string(square) + ": " + oss.str());
            }
            lastMoveTime_ = std::chrono::steady_clock::now();
        } else if (debugEnabled_) {
            logDebug("Mouse down ignored: square=" + std::to_string(square) + ", piece=" + (square != -1 && board.getPieces()[square] ? std::to_string(static_cast<int>(board.getPieces()[square]->getColor())) : "none"));
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
        x = event.button.x;
        y = event.button.y;
        int square = getSquareFromCoords(x, y);
        if (debugEnabled_) {
            logDebug("Mouse up, type: " + std::to_string(event.type) + ", x=" + std::to_string(x) + ", y=" + std::to_string(y) + ", square: " + std::to_string(square));
        }
        if (getSelectedSquare() != -1 && square != -1) {
            int from = getSelectedSquare();
            if (from != square && board.getPieces()[from] && board.getPieces()[from]->getColor() == (isWhiteTurn ? Color::White : Color::Black)) {
                if (board.getPieces()[square] && board.getPieces()[square]->getColor() == board.getPieces()[from]->getColor()) {
                    if (debugEnabled_) logDebug("Invalid move to " + std::to_string(square) + ": cannot capture own piece");
                    setSelectedSquare(-1);
                    isDragging_ = false;
                    dragX_ = -1;
                    dragY_ = -1;
                    updateCursor(false);
                    return;
                }
                Bitboard validMoves = board.getPieces()[from]->generateMoves(board.getOccupied(), from);
                if (debugEnabled_) {
                    std::ostringstream oss;
                    oss << std::hex << validMoves.getValue() << std::dec;
                    logDebug("Checking move from " + std::to_string(from) + " to " + std::to_string(square) + ", valid moves: " + oss.str());
                }
                if (validMoves.getValue() & (1ULL << square)) {
                    if (board.movePiece(static_cast<uint8_t>(from), static_cast<uint8_t>(square))) {
                        if (debugEnabled_) logDebug("Move from " + std::to_string(from) + " to " + std::to_string(square) + " successful");
                        isWhiteTurn = !isWhiteTurn;
                        lastStateChanged = true;
                    } else {
                        if (debugEnabled_) logDebug("Invalid move to " + std::to_string(square) + ": movePiece failed");
                    }
                } else {
                    if (debugEnabled_) logDebug("Invalid move to " + std::to_string(square) + ": not in valid moves list");
                }
            }
        }
        setSelectedSquare(-1);
        isDragging_ = false;
        dragX_ = -1;
        dragY_ = -1;
        updateCursor(false);
    } else if (event.type == SDL_EVENT_QUIT) {
        logDebug("Quit event received");
        quitPromptActive_ = true;
    } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        if (currentTime - lastKeyPressTime_ < 300) return;
        lastKeyPressTime_ = currentTime;
        if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q) {
            logDebug("Escape or Q key pressed");
            quitPromptActive_ = true;
        } else if (quitPromptActive_) {
            if (event.key.key == SDLK_Y) {
                logDebug("Y key pressed, confirming quit");
                quitConfirmed_ = true;
            } else if (event.key.key == SDLK_N) {
                logDebug("N key pressed, canceling quit");
                quitPromptActive_ = false;
            }
        } else if (event.key.key == SDLK_D) {
            logDebug("D key pressed, toggling debug mode");
            debugEnabled_ = !debugEnabled_;
        }
    }
}

int Renderer::getSquareFromCoords(int x, int y) {
    if (x < 50 || x > 750 || y < 100 || y > 600) return -1;
    int file = (x - 50) / 87;
    int rank = 7 - (y - 100) / 62;
    if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
        return rank * 8 + file;
    }
    return -1;
}

void Renderer::updateCursor(bool isDragging) {
    SDL_SetCursor(isDragging ? cursorClosed_ : cursorOpen_);
}

void Renderer::logDebug(const std::string& message) {
    std::cout << message << std::endl;
}

bool Renderer::isKeyPressed(int key) const {
    const Uint8* state = reinterpret_cast<const Uint8*>(SDL_GetKeyboardState(NULL));
    return state && state[key] != 0;
}

void Renderer::closeWindow() {
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}
