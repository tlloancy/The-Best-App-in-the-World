#include "../../include/gui/Renderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>
#include <filesystem>

constexpr int SQUARE_SIZE_X = 87;
constexpr int SQUARE_SIZE_Y = 62;
constexpr int BOARD_OFFSET_X = 50;
constexpr int BOARD_OFFSET_Y = 100;
constexpr int FONT_SIZE = 24;
constexpr int WHITE_TIME_Y = 10;
constexpr int BLACK_TIME_Y = 40;
constexpr int EVAL_BAR_Y = 70;
constexpr int TOP_MOVES_Y = 100;
constexpr int TIME_DISPLAY_HEIGHT = 30;

static SDL_HitTestResult SDLCALL hitTest(SDL_Window* window, const SDL_Point* pt, void* data) {
    Renderer* renderer = static_cast<Renderer*>(data);
    if (pt->x < BOARD_OFFSET_X || pt->x > BOARD_OFFSET_X + 8 * SQUARE_SIZE_X || pt->y < BOARD_OFFSET_Y || pt->y > BOARD_OFFSET_Y + 8 * SQUARE_SIZE_Y) {
        return SDL_HITTEST_DRAGGABLE;
    }
    return SDL_HITTEST_NORMAL;
}

Renderer::Renderer(int width, int height, bool debug) : debugEnabled_(debug), windowWidth_(width), windowHeight_(height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }
    if (TTF_Init() < 0) {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
        return;
    }
    window_ = SDL_CreateWindow("Cosmic Chess", width, height, SDL_WINDOW_RESIZABLE);
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_SetWindowHitTest(window_, hitTest, this);
    SDL_SetWindowMinimumSize(window_, 800, 600);
    renderer_ = SDL_CreateRenderer(window_, NULL);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        return;
    }
    font_ = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", FONT_SIZE);
    if (!font_) {
        logDebug("TTF_OpenFont failed for /usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf: " + std::string(SDL_GetError()));
        font_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", FONT_SIZE);
        if (!font_) {
            logDebug("TTF_OpenFont failed for /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: " + std::string(SDL_GetError()));
        }
    }
    cursorOpen_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    cursorClosed_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
    SDL_SetCursor(cursorOpen_);
    const char* basePath = SDL_GetBasePath();
    basePath_ = basePath ? basePath : "./";
    logDebug("Base path: " + basePath_);
    if (basePath) SDL_free((void*)basePath);
    lastUpdate_ = std::chrono::steady_clock::now();
    lastAIMoveTime_ = std::chrono::steady_clock::now();
    logDebug("Current working directory: " + std::string(std::filesystem::current_path().string()));
    loadPieceTextures();
    logDebug("SDL3 and SDL_image initialized successfully");
    gameOver_ = false;
}

Renderer::~Renderer() {
    if (aiThreadRunning_) {
        aiThreadRunning_ = false;
        if (aiThread_.joinable()) aiThread_.join();
    }
    if (font_) TTF_CloseFont(font_);
    for (auto& pair : textureCache_) SDL_DestroyTexture(pair.second);
    for (auto& pair : evalTextureCache_) SDL_DestroyTexture(pair.second);
    if (cursorOpen_) SDL_DestroyCursor(cursorOpen_);
    if (cursorClosed_) SDL_DestroyCursor(cursorClosed_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
}

bool Renderer::shouldClose() const {
    return quitConfirmed_;
}

void Renderer::updateSearchResult(const Board& board) {
    if (!aiThreadRunning_ && isAIActive_) {
        aiThreadRunning_ = true;
        aiThread_ = std::thread([this, board]() {
            MCTSSearch search;
            SearchResult result = search.search(board, 3);
            std::lock_guard<std::mutex> lock(searchMutex_);
            lastSearchResult_ = result;
            searchResultValid_ = true;
            aiThreadRunning_ = false;
        });
        aiThread_.detach();
    }
}

void Renderer::renderBoard(Board& board, bool& isWhiteTurn) {
    SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
    SDL_RenderClear(renderer_);
    renderChessboard();
    renderPieces(board);
    renderTime(board, isWhiteTurn);
    if (quitPromptActive_) {
        SDL_Color whiteColor = {255, 255, 255, 255};
        renderGameEnd("Quit? Press Y to confirm, N to cancel", whiteColor, true);
    }
    if (board.isCheckmate()) {
        gameOver_ = true;
        std::string winner = isWhiteTurn ? "Black" : "White";
        SDL_Color redColor = {255, 0, 0, 255};
        renderGameEnd(winner + " wins by checkmate! Press N for new game.", redColor, false);
    } else {
        gameOver_ = false;
    }
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - lastUpdate_;
    if (elapsed.count() >= 1.0f) {
        if (isWhiteTurn && !quitPromptActive_) whiteTime_ -= 1;
        else if (!isWhiteTurn && !quitPromptActive_) blackTime_ -= 1;
        lastUpdate_ = now;
    }
    if (whiteTime_ < 0) whiteTime_ = 0;
    if (blackTime_ < 0) blackTime_ = 0;
    if (isAIActive_ && !isWhiteTurn && !searchResultValid_) {
        updateSearchResult(board);
    }
    std::lock_guard<std::mutex> lock(searchMutex_);
    if (searchResultValid_) renderEvaluation(lastSearchResult_);
    auto currentTime = std::chrono::steady_clock::now();
    if (isAIActive_ && !isWhiteTurn && !justAIMoved_ && std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastAIMoveTime_).count() > 1000) {
        std::thread aiMoveThread([this, &board, &isWhiteTurn]() {
            makeAIMove(board, isWhiteTurn);
        });
        aiMoveThread.detach();
        justAIMoved_ = true;
        lastAIMoveTime_ = currentTime;
        searchResultValid_ = false;
    }
    SDL_RenderPresent(renderer_);
}

void Renderer::handleEvents(SDL_Event& event, Board& board, bool& isWhiteTurn) {
    Uint32 currentTime = SDL_GetTicks();
    if (event.type == SDL_EVENT_QUIT) {
        logDebug("Quit event received");
        quitPromptActive_ = true;
    } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        windowWidth_ = event.window.data1;
        windowHeight_ = event.window.data2;
        logDebug("Window resized to " + std::to_string(windowWidth_) + "x" + std::to_string(windowHeight_));
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
        } else if (event.key.key == SDLK_A) {
            isAIActive_ = !isAIActive_;
            logDebug("A key pressed, AI active: " + std::to_string(isAIActive_));
            if (isAIActive_) {
                searchResultValid_ = false;
                if (!isWhiteTurn) updateSearchResult(board);
            } else if (aiThreadRunning_) {
                aiThreadRunning_ = false;
                if (aiThread_.joinable()) aiThread_.join();
            }
        } else if (event.key.key == SDLK_F) {
            isBoardFlipped_ = !isBoardFlipped_;
            logDebug("F key pressed, board flipped: " + std::to_string(isBoardFlipped_));
            searchResultValid_ = false;
        } else if (gameOver_ && event.key.key == SDLK_N) {
            board = Board();
            isWhiteTurn = true;
            gameOver_ = false;
            searchResultValid_ = false;
            whiteTime_ = 600.0f;
            blackTime_ = 600.0f;
            logDebug("New game started");
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT && !gameOver_) {
        int x = event.button.x;
        int y = event.button.y;
        int square = getSquareFromCoords(x, y);
        if (square != -1 && board.getPieces()[square] && board.getPieces()[square]->getColor() == (isWhiteTurn ? Color::White : Color::Black)) {
            setSelectedSquare(square);
            isDragging_ = true;
            dragX_ = x;
            dragY_ = y;
            updateCursor(true);
            if (debugEnabled_) logDebug("Mouse down, type: " + std::to_string(event.type) + ", x=" + std::to_string(x) + ", y=" + std::to_string(y) + ", square: " + std::to_string(square));
            logDebug("Selected square: " + std::to_string(square) + ", piece type: " + std::to_string(static_cast<int>(board.getPieces()[square]->getType())));
            Bitboard validMoves = board.getPieces()[square]->generateMoves(board, square);
            logDebug("Valid moves for " + std::to_string(square) + ": " + std::to_string(validMoves.getValue()));
            justAIMoved_ = false;
        } else if (debugEnabled_) {
            logDebug("Mouse down ignored: square=" + std::to_string(square) + ", piece=" + (square != -1 && board.getPieces()[square] ? std::to_string(static_cast<int>(board.getPieces()[square]->getColor())) : "none"));
        }
    } else if (event.type == SDL_EVENT_MOUSE_MOTION && isDragging_) {
        int x = event.motion.x;
        int y = event.motion.y;
        dragX_ = x;
        dragY_ = y;
        updateCursor(true);
        logDebug("Dragging, mouse at x=" + std::to_string(x) + ", y=" + std::to_string(y));
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT && isDragging_) {
        int x = event.button.x;
        int y = event.button.y;
        int to = getSquareFromCoords(x, y);
        logDebug("Mouse up, type: " + std::to_string(event.type) + ", x=" + std::to_string(x) + ", y=" + std::to_string(y) + ", square: " + std::to_string(to));
        if (to != -1) {
            Bitboard validMoves = board.getPieces()[selectedSquare_]->generateMoves(board, selectedSquare_);
            logDebug("Checking move from " + std::to_string(selectedSquare_) + " to " + std::to_string(to) + ", valid moves: " + std::to_string(validMoves.getValue()));
            if (validMoves.testBit(to) && board.movePiece(selectedSquare_, to)) {
                isWhiteTurn = !isWhiteTurn;
                logDebug("Move from " + std::to_string(selectedSquare_) + " to " + std::to_string(to) + " successful");
                searchResultValid_ = false;
                if (isAIActive_ && !isWhiteTurn) updateSearchResult(board);
            } else {
                logDebug("Invalid move to " + std::to_string(to) + ": " + (validMoves.testBit(to) ? "movePiece failed" : "not in valid moves list"));
            }
        }
        isDragging_ = false;
        dragX_ = -1;
        dragY_ = -1;
        updateCursor(false);
        setSelectedSquare(-1);
    }
}

int Renderer::getSquareFromCoords(int x, int y) const {
    if (x < BOARD_OFFSET_X || x > BOARD_OFFSET_X + 8 * SQUARE_SIZE_X || y < BOARD_OFFSET_Y || y > BOARD_OFFSET_Y + 8 * SQUARE_SIZE_Y) return -1;
    int file = (x - BOARD_OFFSET_X) / SQUARE_SIZE_X;
    int rank = 7 - (y - BOARD_OFFSET_Y) / SQUARE_SIZE_Y;
    if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
        return rank * 8 + file;
    }
    return -1;
}

void Renderer::setSelectedSquare(int square) {
    selectedSquare_ = square;
}

void Renderer::renderChessboard() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            SDL_SetRenderDrawColor(renderer_, (i + j + 1) % 2 ? 200 : 150, (i + j + 1) % 2 ? 180 : 120, (i + j + 1) % 2 ? 160 : 100, 255);
            SDL_FRect rect = {static_cast<float>(BOARD_OFFSET_X + i * SQUARE_SIZE_X), static_cast<float>(BOARD_OFFSET_Y + j * SQUARE_SIZE_Y), static_cast<float>(SQUARE_SIZE_X), static_cast<float>(SQUARE_SIZE_Y)};
            SDL_RenderFillRect(renderer_, &rect);
        }
    }
    SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
    SDL_FRect borderRect = {static_cast<float>(BOARD_OFFSET_X), static_cast<float>(BOARD_OFFSET_Y), static_cast<float>(8 * SQUARE_SIZE_X), static_cast<float>(8 * SQUARE_SIZE_Y)};
    SDL_RenderRect(renderer_, &borderRect);
}

void Renderer::loadPieceTextures() {
    const std::string pieces[6] = {"pawn", "knight", "bishop", "rook", "queen", "king"};
    const std::string colors[2] = {"white", "black"};
    for (const auto& piece : pieces) {
        for (const auto& color : colors) {
            std::string path = basePath_ + "../assets/" + color + "_" + piece + ".png";
            SDL_Surface* surface = IMG_Load(path.c_str());
            if (!surface) {
                std::cerr << "IMG_Load failed for " << path << ": " << SDL_GetError() << std::endl;
                continue;
            }
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
            SDL_DestroySurface(surface);
            if (!texture) {
                std::cerr << "SDL_CreateTextureFromSurface failed for " << path << ": " << SDL_GetError() << std::endl;
                continue;
            }
            PieceType type = static_cast<PieceType>(std::distance(std::begin(pieces), std::find(std::begin(pieces), std::end(pieces), piece)));
            Color col = (color == "white") ? Color::White : Color::Black;
            textureCache_[{type, col}] = texture;
        }
    }
}

void Renderer::renderPieces(const Board& board) {
    for (int idx = 0; idx < 64; ++idx) {
        if (board.getPieces()[idx] && idx != selectedSquare_) {
            int file = idx % 8;
            int rank = 7 - (idx / 8);
            SDL_FRect rect = {static_cast<float>(BOARD_OFFSET_X + file * SQUARE_SIZE_X + 18), static_cast<float>(BOARD_OFFSET_Y + rank * SQUARE_SIZE_Y + 6), 50.0f, 50.0f};
            SDL_RenderTexture(renderer_, textureCache_[{board.getPieces()[idx]->getType(), board.getPieces()[idx]->getColor()}], NULL, &rect);
        }
    }
    if (isDragging_ && selectedSquare_ != -1 && board.getPieces()[selectedSquare_]) {
        SDL_FRect rect = {static_cast<float>(dragX_ - 25), static_cast<float>(dragY_ - 25), 50.0f, 50.0f};
        SDL_RenderTexture(renderer_, textureCache_[{board.getPieces()[selectedSquare_]->getType(), board.getPieces()[selectedSquare_]->getColor()}], NULL, &rect);
    }
}

void Renderer::renderTime(const Board& board, bool isWhiteTurn) {
    if (!font_) return;
    std::string whiteTimeStr = "White: " + std::to_string(static_cast<int>(whiteTime_)) + "s";
    std::string blackTimeStr = "Black: " + std::to_string(static_cast<int>(blackTime_)) + "s";
    SDL_Color whiteColor = {255, 255, 255, 255};
    SDL_Surface* whiteSurface = TTF_RenderText_Blended(font_, whiteTimeStr.c_str(), whiteTimeStr.length(), whiteColor);
    SDL_Surface* blackSurface = TTF_RenderText_Blended(font_, blackTimeStr.c_str(), blackTimeStr.length(), whiteColor);
    if (whiteSurface && blackSurface) {
        SDL_Texture* whiteTexture = SDL_CreateTextureFromSurface(renderer_, whiteSurface);
        SDL_Texture* blackTexture = SDL_CreateTextureFromSurface(renderer_, blackSurface);
        if (whiteTexture && blackTexture) {
            SDL_FRect whiteRect = {10.0f, WHITE_TIME_Y, static_cast<float>(whiteSurface->w), static_cast<float>(whiteSurface->h)};
            SDL_FRect blackRect = {10.0f, BLACK_TIME_Y, static_cast<float>(blackSurface->w), static_cast<float>(blackSurface->h)};
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

void Renderer::renderGameEnd(const std::string& message, const SDL_Color& color, bool isQuitPrompt) {
    if (!font_) return;
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

void Renderer::renderEvaluation(const SearchResult& result) {
    if (!font_) return;
    std::string evalStr;
    if (result.score >= 1000.0f) evalStr = "M" + std::to_string(static_cast<int>(result.score / 1000));
    else if (result.score <= -1000.0f) evalStr = "-M" + std::to_string(static_cast<int>(-result.score / 1000));
    else evalStr = (result.score >= 0 ? "+" : "") + std::to_string(result.score).substr(0, 4);
    SDL_Color whiteColor = {255, 255, 255, 255};
    SDL_Surface* evalSurface = TTF_RenderText_Blended(font_, evalStr.c_str(), evalStr.length(), whiteColor);
    if (evalSurface) {
        SDL_Texture* evalTexture = SDL_CreateTextureFromSurface(renderer_, evalSurface);
        if (evalTexture) {
            SDL_FRect evalRect = {50.0f, EVAL_BAR_Y, static_cast<float>(evalSurface->w), static_cast<float>(evalSurface->h)};
            SDL_RenderTexture(renderer_, evalTexture, NULL, &evalRect);
            SDL_DestroyTexture(evalTexture);
        }
        SDL_DestroySurface(evalSurface);
    }
    SDL_SetRenderDrawColor(renderer_, result.score > 0 ? 50 : 200, result.score > 0 ? 200 : 50, 100, 100);
    SDL_FRect bar = {50.0f, EVAL_BAR_Y + 20, 700.0f * std::min(std::abs(result.score) / 10.0f, 1.0f), 20.0f};
    SDL_RenderFillRect(renderer_, &bar);
    for (size_t i = 0; i < result.topMoves.size() && i < 3; ++i) {
        std::string moveStr = std::string(1, 'a' + (result.topMoves[i].from % 8)) + std::to_string(8 - (result.topMoves[i].from / 8)) +
                              "-" + std::string(1, 'a' + (result.topMoves[i].to % 8)) + std::to_string(8 - (result.topMoves[i].to / 8));
        SDL_Surface* moveSurface = TTF_RenderText_Blended(font_, moveStr.c_str(), moveStr.length(), whiteColor);
        if (moveSurface) {
            SDL_Texture* moveTexture = SDL_CreateTextureFromSurface(renderer_, moveSurface);
            if (moveTexture) {
                SDL_FRect moveRect = {50.0f, TOP_MOVES_Y + i * 20.0f, static_cast<float>(moveSurface->w), static_cast<float>(moveSurface->h)};
                SDL_RenderTexture(renderer_, moveTexture, NULL, &moveRect);
                SDL_DestroyTexture(moveTexture);
            }
            SDL_DestroySurface(moveSurface);
        }
    }
}

void Renderer::makeAIMove(Board& board, bool& isWhiteTurn) {
    Color color = isWhiteTurn ? Color::White : Color::Black;
    MCTSSearch search;
    SearchResult result = search.search(board, 3);
    if (result.bestMove.from != result.bestMove.to) {
        if (board.movePiece(result.bestMove.from, result.bestMove.to)) {
            isWhiteTurn = !isWhiteTurn;
            logDebug("AI move from " + std::to_string(result.bestMove.from) + " to " + std::to_string(result.bestMove.to));
        }
    }
}

void Renderer::updateCursor(bool isDragging) {
    SDL_SetCursor(isDragging ? cursorClosed_ : cursorOpen_);
}

void Renderer::logDebug(const std::string& message) const {
    if (debugEnabled_) std::cout << message << std::endl;
}
