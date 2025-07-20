#include "../../include/gui/Renderer.hpp"
#include "../../include/utils/Convert.hpp"
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
#include <cmath>

#define AI_DEBUG
#ifdef AI_DEBUG
#define AI_LOG(msg) std::cerr << "[AI_DEBUG] " << msg << std::endl
#else
#define AI_LOG(msg)
#endif

constexpr int SQUARE_SIZE_X = 87;
constexpr int SQUARE_SIZE_Y = 62;
constexpr int BOARD_OFFSET_X = 50;
constexpr int BOARD_OFFSET_Y = 100;
constexpr int FONT_SIZE = 24;
constexpr int BOLD_FONT_SIZE = 48;
constexpr int SMALL_FONT_SIZE = 16;
constexpr int WHITE_TIME_Y = 10;
constexpr int BLACK_TIME_Y = 40;
constexpr int EVAL_BAR_X = 800;
constexpr int EVAL_BAR_Y = 100;
constexpr int TIME_DISPLAY_HEIGHT = 30;
constexpr int NAV_BUTTON_Y = 600;
constexpr int NAV_BUTTON_W = 50;
constexpr int NAV_BUTTON_H = 30;
constexpr int NAV_BUTTON_SPACING = 10;
constexpr int WINDOW_WIDTH = 1280;
constexpr int VARIANTS_X = 850;
constexpr int VARIANTS_Y = 100;

static SDL_HitTestResult SDLCALL hitTest(SDL_Window* window, const SDL_Point* pt, void* data) {
    Renderer* renderer = static_cast<Renderer*>(data);
    if (pt->x < BOARD_OFFSET_X || pt->x > BOARD_OFFSET_X + 8 * SQUARE_SIZE_X || pt->y < BOARD_OFFSET_Y || pt->y > BOARD_OFFSET_Y + 8 * SQUARE_SIZE_Y) {
        if (pt->y >= NAV_BUTTON_Y && pt->y <= NAV_BUTTON_Y + NAV_BUTTON_H && pt->x >= 300 && pt->x <= 300 + 4 * (NAV_BUTTON_W + NAV_BUTTON_SPACING)) {
            return SDL_HITTEST_NORMAL;
        }
        return SDL_HITTEST_DRAGGABLE;
    }
    return SDL_HITTEST_NORMAL;
}

void Renderer::renderLoadingScreen(float progress) {
    SDL_SetRenderDrawColor(renderer_, 10, 10, 30, 255);
    SDL_RenderClear(renderer_);
    Uint32 time = SDL_GetTicks();
    float t = time / 1000.0f;
    for (const auto& star : stars_) {
        Uint8 alpha = 180 + 75 * std::sin(t * 3 + star.x + star.y);
        SDL_SetRenderDrawColor(renderer_, 200, 200, 255, alpha);
        SDL_FRect starRect = {star.x, star.y, 2.0f, 2.0f};
        SDL_RenderFillRect(renderer_, &starRect);
    }
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 20, 20, 60, 180);
    SDL_FRect gradientRect = {0.0f, 0.0f, static_cast<float>(windowWidth_), static_cast<float>(windowHeight_)};
    SDL_RenderFillRect(renderer_, &gradientRect);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
    SDL_Color neonColor = {100, 255, 200, static_cast<Uint8>(180 + 75 * std::sin(t * 2))};
    std::string loadingText = "Initializing AI... " + std::to_string(static_cast<int>(progress * 100)) + "%";
    SDL_Surface* surface = TTF_RenderText_Blended(boldFont_, loadingText.c_str(), loadingText.length(), neonColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        if (texture) {
            SDL_SetTextureAlphaMod(texture, neonColor.a);
            SDL_FRect textRect = {static_cast<float>(windowWidth_ / 2 - surface->w / 2), 300.0f, static_cast<float>(surface->w), static_cast<float>(surface->h)};
            SDL_FRect shadowRect = {textRect.x + 4, textRect.y + 4, textRect.w, textRect.h};
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 100);
            SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
            SDL_RenderTexture(renderer_, texture, NULL, &shadowRect);
            SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
            SDL_RenderTexture(renderer_, texture, NULL, &textRect);
            SDL_DestroyTexture(texture);
        } else {
            logDebug("Failed to create texture for loading screen: " + std::string(SDL_GetError()));
        }
        SDL_DestroySurface(surface);
    } else {
        logDebug("Failed to render loading text: " + std::string(SDL_GetError()));
    }
    SDL_RenderPresent(renderer_);
}

Renderer::Renderer(int width, int height, bool debug, bool enableMenu) : debugEnabled_(debug), enableMenu_(enableMenu), windowWidth_(width), windowHeight_(height) {
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
    boldFont_ = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf", BOLD_FONT_SIZE);
    smallFont_ = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", SMALL_FONT_SIZE);
    if (!font_) {
        logDebug("TTF_OpenFont failed for /usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf: " + std::string(SDL_GetError()));
        font_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", FONT_SIZE);
        if (!font_) {
            logDebug("TTF_OpenFont failed for /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: " + std::string(SDL_GetError()));
        }
    }
    if (!boldFont_) {
        logDebug("TTF_OpenFont failed for /usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf: " + std::string(SDL_GetError()));
        boldFont_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", BOLD_FONT_SIZE);
        if (!boldFont_) {
            logDebug("TTF_OpenFont failed for /usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf: " + std::string(SDL_GetError()));
            boldFont_ = font_;
        }
    }
    if (!smallFont_) {
        logDebug("TTF_OpenFont failed for /usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf: " + std::string(SDL_GetError()));
        smallFont_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", SMALL_FONT_SIZE);
        if (!smallFont_) {
            logDebug("TTF_OpenFont failed for /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf: " + std::string(SDL_GetError()));
            smallFont_ = font_;
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
    moveHistory_.push_back(Board());
    historyIndex_ = 0;
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> distX(0.0f, static_cast<float>(windowWidth_));
    std::uniform_real_distribution<float> distY(0.0f, static_cast<float>(windowHeight_));
    for (int i = 0; i < 50; ++i) {
        stars_.push_back({distX(gen), distY(gen)});
    }
    renderLoadingScreen(0.0f);
    auto start = std::chrono::steady_clock::now();
    float estimatedDuration = 5.0f;
    aiInitialized_ = false;
    std::thread initThread([&]() {
        try {
            mctsSearch_ = new MCTSSearch(1500, 0);
            AI_LOG("MCTSSearch initialized in Renderer constructor with Elo 1500, think time 0ms");
            aiInitialized_ = true;
        } catch (const std::exception& e) {
            AI_LOG("Failed to initialize MCTSSearch: " + std::string(e.what()));
            mctsSearch_ = nullptr;
            aiInitialized_ = true;
        }
    });
    while (!aiInitialized_) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - start;
        float progress = std::min(elapsed.count() / estimatedDuration, 1.0f);
        renderLoadingScreen(progress);
        SDL_Delay(50);
        if (aiInitialized_ || elapsed.count() >= 10.0f) {
            if (initThread.joinable()) {
                initThread.join();
                if (!mctsSearch_) {
                    AI_LOG("MCTSSearch initialization timed out or failed, using non-Stockfish search");
                    useStockfish_ = false;
                }
                AI_LOG("AI initialization completed in " + std::to_string(elapsed.count()) + " seconds");
                renderLoadingScreen(1.0f);
                break;
            }
        }
    }
    if (enableMenu_) {
        menu_ = new Menu(renderer_, font_, boldFont_, smallFont_);
        gameState_ = GameState::Menu;
    } else {
        gameState_ = GameState::Board;
    }
}

Renderer::~Renderer() {
    if (aiThreadRunning_) {
        aiThreadRunning_ = false;
        if (aiThread_.joinable()) aiThread_.join();
    }
    if (mctsSearch_) delete mctsSearch_;
    if (menu_) delete menu_;
    if (font_) TTF_CloseFont(font_);
    if (boldFont_) TTF_CloseFont(boldFont_);
    if (smallFont_) TTF_CloseFont(smallFont_);
    for (auto& pair : textureCache_) SDL_DestroyTexture(pair.second);
    for (auto& pair : evalTextureCache_) SDL_DestroyTexture(pair.second);
    if (cursorOpen_) SDL_DestroyCursor(cursorOpen_);
    if (cursorClosed_) SDL_DestroyCursor(cursorClosed_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool Renderer::shouldClose() const {
    return quitConfirmed_;
}

void Renderer::updateSearchResult(const Board& board, bool isWhiteTurn) {
    if (!aiThreadRunning_ && isAIActive_ && !isWhiteTurn && !justAIMoved_) {
        AI_LOG("Starting AI search thread, using Stockfish: " + std::to_string(useStockfish_));
        if (aiThread_.joinable()) {
            aiThreadRunning_ = false;
            try {
                aiThread_.join();
                AI_LOG("Previous AI thread joined");
            } catch (const std::exception& e) {
                AI_LOG("Exception in joining AI thread: " + std::string(e.what()));
            }
        }
        aiThreadRunning_ = true;
        moveHistory_.push_back(board);
        historyIndex_ = moveHistory_.size() - 1;
        aiThread_ = std::thread([this, board, isWhiteTurn]() {
            try {
                SearchResult result;
                std::string uciMove;
                if (useStockfish_) {
                    AI_LOG("Launching StockfishSearch for depth 10");
                    StockfishSearch search(10);
                    result = search.search(board, 10, &uciMove);
                } else if (mctsSearch_) {
                    AI_LOG("Launching MCTSSearch for depth 3");
                    result = mctsSearch_->search(board, 3, &uciMove);
                } else {
                    AI_LOG("MCTSSearch is null, cannot perform search");
                    result.score = 0.0f;
                    result.bestMove = {0, 0};
                    result.topMoves.clear();
                    aiThreadRunning_ = false;
                    return;
                }
                if (std::isnan(result.score) || std::isinf(result.score)) {
                    AI_LOG("Invalid search result score: " + std::to_string(result.score));
                    result.score = 0.0f;
                }
                std::lock_guard<std::mutex> lock(searchMutex_);
                float adjustedScore = -result.score;
                result.score = adjustedScore;
                lastSearchResult_ = result;
                searchResultValid_ = true;
                aiThreadRunning_ = false;
                AI_LOG("AI search completed, adjusted score: " + std::to_string(adjustedScore) + ", best move: " + uciMove + ", top moves: " + std::to_string(result.topMoves.size()));
            } catch (const std::exception& e) {
                AI_LOG("Exception in AI search thread: " + std::string(e.what()));
                std::lock_guard<std::mutex> lock(searchMutex_);
                lastSearchResult_.score = 0.0f;
                lastSearchResult_.bestMove = {0, 0};
                lastSearchResult_.topMoves.clear();
                searchResultValid_ = false;
                aiThreadRunning_ = false;
            }
        });
    }
}

void Renderer::renderBoard(Board& board, bool& isWhiteTurn) {
    SDL_SetRenderDrawColor(renderer_, 30, 64, 138, 255); // #1E3A8A
    SDL_RenderClear(renderer_);
    Uint32 time = SDL_GetTicks();
    float t = time / 1000.0f;
    if (enableMenu_ && gameState_ == GameState::Menu) {
        menu_->render();
        SDL_RenderPresent(renderer_);
        return;
    }
    for (const auto& star : stars_) {
        Uint8 alpha = 100 + 155 * (0.5f + 0.5f * std::sin(t * 2.0f + star.x + star.y));
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, alpha);
        SDL_FRect starRect = {star.x, star.y, 1.0f, 1.0f};
        SDL_RenderFillRect(renderer_, &starRect);
    }
    renderChessboard();
    if (historyIndex_ >= 0 && historyIndex_ < static_cast<int>(moveHistory_.size()) && !isDragging_) {
        renderPieces(moveHistory_[historyIndex_]);
    } else {
        renderPieces(board);
    }
    renderTime(board, isWhiteTurn);
    renderNavigationButtons();
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
    if (elapsed.count() >= 1.0f && !gameOver_ && !quitPromptActive_) {
        if (isWhiteTurn) whiteTime_ -= 1;
        else blackTime_ -= 1;
        lastUpdate_ = now;
    }
    if (whiteTime_ < 0) whiteTime_ = 0;
    if (blackTime_ < 0) blackTime_ = 0;
    if (showEvaluation_) {
        renderEvaluation(lastSearchResult_);
    }
    if (isAIActive_ && !isWhiteTurn && !aiThreadRunning_ && !searchResultValid_ && !justAIMoved_) {
        updateSearchResult(board, isWhiteTurn);
    }
    if (searchResultValid_ && isAIActive_ && !isWhiteTurn && !justAIMoved_) {
        AI_LOG("Applying AI move in main thread");
        makeAIMove(board, isWhiteTurn);
        justAIMoved_ = true;
        lastAIMoveTime_ = now;
        searchResultValid_ = false;
        renderChessboard();
        renderPieces(board);
        SDL_RenderPresent(renderer_);
    }
    if (premove_.first != -1 && premove_.second != -1 && isWhiteTurn && !aiThreadRunning_) {
        Bitboard validMoves = board.getPieces()[premove_.first] ? board.getPieces()[premove_.first]->generateMoves(board, premove_.first) : Bitboard(0);
        if (validMoves.testBit(premove_.second)) {
            if (board.movePiece(premove_.first, premove_.second)) {
                AI_LOG("Premove from " + std::to_string(premove_.first) + " to " + std::to_string(premove_.second) + " applied");
                isWhiteTurn = !isWhiteTurn;
                moveHistory_.push_back(board);
                historyIndex_ = moveHistory_.size() - 1;
                premove_ = {-1, -1};
                justAIMoved_ = false;
                renderChessboard();
                renderPieces(board);
                SDL_RenderPresent(renderer_);
                if (isAIActive_ && !isWhiteTurn && !aiThreadRunning_) {
                    AI_LOG("Triggering AI search after premove");
                    updateSearchResult(board, isWhiteTurn);
                }
            } else {
                AI_LOG("Premove from " + std::to_string(premove_.first) + " to " + std::to_string(premove_.second) + " failed: movePiece returned false");
            }
        } else {
            AI_LOG("Premove from " + std::to_string(premove_.first) + " to " + std::to_string(premove_.second) + " invalid, bitboard: " + std::to_string(validMoves.getValue()));
        }
        premove_ = {-1, -1};
    }
    SDL_RenderPresent(renderer_);
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

void Renderer::logDebug(const std::string& message) const {
    if (debugEnabled_) std::cout << message << std::endl;
}

void Renderer::updateCursor(bool isDragging) {
    SDL_SetCursor(isDragging ? cursorClosed_ : cursorOpen_);
}

void Renderer::renderChessboard() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if ((i + j) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer_, 59, 130, 246, 255); // #3B82F6
            } else {
                SDL_SetRenderDrawColor(renderer_, 30, 64, 175, 255); // #1E40AF
            }
            SDL_FRect rect = {static_cast<float>(BOARD_OFFSET_X + i * SQUARE_SIZE_X), static_cast<float>(BOARD_OFFSET_Y + j * SQUARE_SIZE_Y), static_cast<float>(SQUARE_SIZE_X), static_cast<float>(SQUARE_SIZE_Y)};
            SDL_RenderFillRect(renderer_, &rect);
        }
    }
    SDL_SetRenderDrawColor(renderer_, 209, 213, 219, 255); // #D1D5DB
    SDL_FRect borderRect = {static_cast<float>(BOARD_OFFSET_X), static_cast<float>(BOARD_OFFSET_Y), static_cast<float>(8 * SQUARE_SIZE_X), static_cast<float>(8 * SQUARE_SIZE_Y)};
    SDL_RenderRect(renderer_, &borderRect);
    if (selectedSquare_ != -1) {
        Uint32 time = SDL_GetTicks();
        float t = time / 1000.0f;
        int file = selectedSquare_ % 8;
        int rank = 7 - (selectedSquare_ / 8);
        Uint8 alpha = 100 + 80 * (0.5f + 0.5f * std::sin(t * 2));
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, 255, 255, 0, alpha);
        SDL_FRect glowRect = {static_cast<float>(BOARD_OFFSET_X + file * SQUARE_SIZE_X - 5), static_cast<float>(BOARD_OFFSET_Y + rank * SQUARE_SIZE_Y - 5), static_cast<float>(SQUARE_SIZE_X + 10), static_cast<float>(SQUARE_SIZE_Y + 10)};
        SDL_RenderFillRect(renderer_, &glowRect);
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
    }
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
    if (!font_) {
        logDebug("Font is null in renderTime");
        return;
    }
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
    if (!font_) {
        logDebug("Font is null in renderGameEnd");
        return;
    }
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
    if (!font_ || !boldFont_ || !smallFont_) {
        logDebug("Font is null in renderEvaluation, skipping render");
        return;
    }
    if (std::isnan(result.score) || std::isinf(result.score)) {
        AI_LOG("Skipping evaluation render due to invalid score: " + std::to_string(result.score));
        return;
    }
    static float displayedScore = 0.0f;
    static float lastLoggedScore = std::numeric_limits<float>::quiet_NaN();
    displayedScore += (result.score - displayedScore) * 0.05f;
    if (std::isnan(displayedScore) || std::isinf(displayedScore)) {
        AI_LOG("Invalid displayed score: " + std::to_string(displayedScore));
        displayedScore = 0.0f;
    }
    if (std::abs(displayedScore - lastLoggedScore) > 0.01) {
        AI_LOG("Rendering evaluation: raw score=" + std::to_string(result.score) + ", displayed=" + std::to_string(displayedScore));
        lastLoggedScore = displayedScore;
    }
    std::ostringstream evalStream;
    if (std::abs(displayedScore) >= 1000.0f) {
        int mateDistance = static_cast<int>(std::abs(displayedScore)) - 1000 + 1;
        evalStream << (displayedScore >= 0 ? "M" : "-M") << mateDistance;
    } else if (std::abs(displayedScore) >= 30.0f) {
        evalStream << (displayedScore >= 0 ? "M1" : "-M1");
    } else {
        evalStream << std::fixed << std::setprecision(2) << (displayedScore >= 0 ? "+" : "") << displayedScore;
    }
    std::string evalStr = evalStream.str();
    SDL_Color neonColor = {static_cast<Uint8>(displayedScore < 0 ? 0 : 100), static_cast<Uint8>(displayedScore < 0 ? 0 : 255), static_cast<Uint8>(displayedScore < 0 ? 0 : 200), static_cast<Uint8>(180 + 75 * std::sin(SDL_GetTicks() / 1000.0f * 2))};
    if (displayedScore < 0) neonColor = {0, 0, 0, 255};
    SDL_Surface* evalSurface = TTF_RenderText_Blended(boldFont_, evalStr.c_str(), evalStr.length(), neonColor);
    if (evalSurface) {
        SDL_Texture* evalTexture = SDL_CreateTextureFromSurface(renderer_, evalSurface);
        if (evalTexture) {
            SDL_SetTextureAlphaMod(evalTexture, neonColor.a);
            SDL_FRect evalRect = {static_cast<float>(EVAL_BAR_X), static_cast<float>(BOARD_OFFSET_Y + 8 * SQUARE_SIZE_Y + 10), static_cast<float>(evalSurface->w), static_cast<float>(evalSurface->h)};
            SDL_RenderTexture(renderer_, evalTexture, NULL, &evalRect);
            SDL_DestroyTexture(evalTexture);
        } else {
            logDebug("Failed to create texture for evaluation: " + std::string(SDL_GetError()));
        }
        SDL_DestroySurface(evalSurface);
    } else {
        logDebug("Failed to render evaluation text: " + std::string(SDL_GetError()));
    }
    float eval = displayedScore;
    const float MAX_EVAL_RANGE = 10.0f;
    float splitPoint = 248.0f - (eval / MAX_EVAL_RANGE) * 248.0f;
    splitPoint = std::clamp(splitPoint, 0.0f, 496.0f);
    static float lastLoggedSplit = std::numeric_limits<float>::quiet_NaN();
    if (std::abs(splitPoint - lastLoggedSplit) > 0.01f) {
        AI_LOG("Eval: " + std::to_string(eval) + ", Split point: " + std::to_string(splitPoint));
        lastLoggedSplit = splitPoint;
    }
    SDL_SetRenderDrawColor(renderer_, 75, 85, 100, 255); // #4B5564 neutral background
    SDL_FRect bgBar = {static_cast<float>(EVAL_BAR_X), static_cast<float>(BOARD_OFFSET_Y), 20.0f, 496.0f};
    SDL_RenderFillRect(renderer_, &bgBar);
    if (!isBoardFlipped_) {
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255); // Black
        SDL_FRect blackBar = {static_cast<float>(EVAL_BAR_X), static_cast<float>(BOARD_OFFSET_Y), 20.0f, splitPoint};
        SDL_RenderFillRect(renderer_, &blackBar);
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255); // White
        SDL_FRect whiteBar = {static_cast<float>(EVAL_BAR_X), static_cast<float>(BOARD_OFFSET_Y + splitPoint), 20.0f, 496.0f - splitPoint};
        SDL_RenderFillRect(renderer_, &whiteBar);
    } else {
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255); // White
        SDL_FRect whiteBar = {static_cast<float>(EVAL_BAR_X), static_cast<float>(BOARD_OFFSET_Y), 20.0f, splitPoint};
        SDL_RenderFillRect(renderer_, &whiteBar);
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255); // Black
        SDL_FRect blackBar = {static_cast<float>(EVAL_BAR_X), static_cast<float>(BOARD_OFFSET_Y + splitPoint), 20.0f, 496.0f - splitPoint};
        SDL_RenderFillRect(renderer_, &blackBar);
    }
    int yOffset = VARIANTS_Y;
    for (size_t i = 0; i < std::min<size_t>(result.topMoves.size(), 10); ++i) {
        const auto& move = result.topMoves[i];
        std::string moveStr = std::string(1, 'a' + (move.first % 8)) + std::to_string(8 - (move.first / 8)) +
                              "-" + std::string(1, 'a' + (move.second % 8)) + std::to_string(8 - (move.second / 8));
        SDL_Surface* moveSurface = TTF_RenderText_Blended(smallFont_, moveStr.c_str(), moveStr.length(), neonColor);
        if (moveSurface) {
            SDL_Texture* moveTexture = SDL_CreateTextureFromSurface(renderer_, moveSurface);
            if (moveTexture) {
                SDL_SetTextureAlphaMod(moveTexture, neonColor.a);
                SDL_FRect moveRect = {static_cast<float>(VARIANTS_X), static_cast<float>(yOffset), static_cast<float>(moveSurface->w), static_cast<float>(moveSurface->h)};
                SDL_RenderTexture(renderer_, moveTexture, NULL, &moveRect);
                SDL_DestroyTexture(moveTexture);
            } else {
                logDebug("Failed to create texture for move variation: " + std::string(SDL_GetError()));
            }
            SDL_DestroySurface(moveSurface);
        } else {
            logDebug("Failed to render move variation text: " + std::string(SDL_GetError()));
        }
        yOffset += 20;
    }
}

void Renderer::renderNavigationButtons() {
    if (!font_) {
        logDebug("Font is null in renderNavigationButtons");
        return;
    }
    SDL_Color buttonColor = {59, 130, 246, 255}; // #3B82F6
    SDL_Color textColor = {255, 255, 255, 255};
    const char* buttons[] = {"<<", "<", ">", ">>"};
    int buttonX = 300;
    for (int i = 0; i < 4; ++i) {
        SDL_FRect buttonRect = {static_cast<float>(buttonX + i * (NAV_BUTTON_W + NAV_BUTTON_SPACING)), static_cast<float>(NAV_BUTTON_Y), static_cast<float>(NAV_BUTTON_W), static_cast<float>(NAV_BUTTON_H)};
        SDL_SetRenderDrawColor(renderer_, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        SDL_RenderFillRect(renderer_, &buttonRect);
        SDL_Surface* surface = TTF_RenderText_Blended(font_, buttons[i], strlen(buttons[i]), textColor);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
            if (texture) {
                SDL_FRect textRect = {buttonRect.x + 10, buttonRect.y + 5, static_cast<float>(surface->w), static_cast<float>(surface->h)};
                SDL_RenderTexture(renderer_, texture, NULL, &textRect);
                SDL_DestroyTexture(texture);
            } else {
                logDebug("Failed to create texture for navigation button: " + std::string(SDL_GetError()));
            }
            SDL_DestroySurface(surface);
        } else {
            logDebug("Failed to render navigation button text: " + std::string(SDL_GetError()));
        }
    }
}

bool Renderer::isPointInButton(int x, int y, int buttonX, int buttonY, int buttonW, int buttonH) const {
    bool inButton = x >= buttonX && x <= buttonX + buttonW && y >= buttonY && y <= buttonY + buttonH;
    if (inButton) {
        AI_LOG("Button click at x=" + std::to_string(x) + ", y=" + std::to_string(y) + ", buttonX=" + std::to_string(buttonX) + ", buttonY=" + std::to_string(buttonY));
    }
    return inButton;
}

void Renderer::makeAIMove(Board& board, bool& isWhiteTurn) {
    if (!searchResultValid_) {
        AI_LOG("No valid search result for AI move");
        return;
    }
    AI_LOG("Attempting AI move from " + std::to_string(lastSearchResult_.bestMove.first) + " to " + std::to_string(lastSearchResult_.bestMove.second));
    if (!board.getPieces()[lastSearchResult_.bestMove.first]) {
        AI_LOG("AI move failed: no piece at from position " + std::to_string(lastSearchResult_.bestMove.first));
        return;
    }
    AI_LOG("Piece at from position " + std::to_string(lastSearchResult_.bestMove.first) + " is valid");
    if (board.getPieces()[lastSearchResult_.bestMove.first]->getColor() != Color::Black) {
        AI_LOG("AI move failed: piece at from position " + std::to_string(lastSearchResult_.bestMove.first) + " is not black");
        return;
    }
    AI_LOG("Piece color at from position is black");
    Bitboard validMoves = board.getPieces()[lastSearchResult_.bestMove.first]->generateMoves(board, lastSearchResult_.bestMove.first);
    if (!validMoves.testBit(lastSearchResult_.bestMove.second)) {
        AI_LOG("AI move failed: move from " + std::to_string(lastSearchResult_.bestMove.first) + " to " + std::to_string(lastSearchResult_.bestMove.second) + " is not valid, bitboard: " + std::to_string(validMoves.getValue()));
        return;
    }
    AI_LOG("Move from " + std::to_string(lastSearchResult_.bestMove.first) + " to " + std::to_string(lastSearchResult_.bestMove.second) + " is valid");
    if (board.movePiece(lastSearchResult_.bestMove.first, lastSearchResult_.bestMove.second)) {
        isWhiteTurn = !isWhiteTurn;
        AI_LOG("AI move from " + std::to_string(lastSearchResult_.bestMove.first) + " to " + std::to_string(lastSearchResult_.bestMove.second) + " successful");
        moveHistory_.push_back(board);
        historyIndex_ = moveHistory_.size() - 1;
    } else {
        AI_LOG("AI move from " + std::to_string(lastSearchResult_.bestMove.first) + " to " + std::to_string(lastSearchResult_.bestMove.second) + " failed: movePiece returned false");
    }
}

void Renderer::handleEvents(SDL_Event& event, Board& board, bool& isWhiteTurn) {
    if (enableMenu_ && gameState_ == GameState::Menu) {
        if (menu_->handleEvents(event)) {
            if (mctsSearch_) {
                mctsSearch_->setSkillLevel(menu_->getElo(), menu_->getThinkTime());
                useStockfish_ = (menu_->getElo() >= 1500);
                AI_LOG("Menu completed: elo=" + std::to_string(menu_->getElo()) + ", thinkTime=" + std::to_string(menu_->getThinkTime()) + "ms, useStockfish=" + std::to_string(useStockfish_));
            }
            gameState_ = GameState::Board;
        }
        return;
    }
    if (event.type == SDL_EVENT_QUIT) {
        logDebug("Quit event received");
        quitPromptActive_ = true;
    } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
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
            AI_LOG("A key pressed, AI active: " + std::to_string(isAIActive_));
            if (isAIActive_ && !isWhiteTurn && !aiThreadRunning_) {
                searchResultValid_ = false;
                AI_LOG("Triggering AI search for black's turn");
                updateSearchResult(board, isWhiteTurn);
            } else if (!isAIActive_ && aiThreadRunning_) {
                aiThreadRunning_ = false;
                if (aiThread_.joinable()) aiThread_.join();
            }
        } else if (event.key.key == SDLK_F) {
            isBoardFlipped_ = !isBoardFlipped_;
            AI_LOG("F key pressed, board flipped: " + std::to_string(isBoardFlipped_));
        } else if (event.key.key == SDLK_E) {
            showEvaluation_ = !showEvaluation_;
            AI_LOG("E key pressed, show evaluation: " + std::to_string(showEvaluation_));
        } else if (gameOver_ && event.key.key == SDLK_N) {
            board = Board();
            isWhiteTurn = true;
            gameOver_ = false;
            searchResultValid_ = false;
            whiteTime_ = 600.0f;
            blackTime_ = 600.0f;
            moveHistory_.clear();
            moveHistory_.push_back(board);
            historyIndex_ = 0;
            premove_ = {-1, -1};
            AI_LOG("New game started");
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT && !gameOver_ && !quitPromptActive_) {
        int x = event.button.x;
        int y = event.button.y;
        if (isPointInButton(x, y, 300, NAV_BUTTON_Y, NAV_BUTTON_W, NAV_BUTTON_H)) {
            historyIndex_ = 0;
            AI_LOG("Navigated to first move, history size: " + std::to_string(moveHistory_.size()));
            renderChessboard();
            renderPieces(moveHistory_[historyIndex_]);
            SDL_RenderPresent(renderer_);
        } else if (isPointInButton(x, y, 300 + NAV_BUTTON_W + NAV_BUTTON_SPACING, NAV_BUTTON_Y, NAV_BUTTON_W, NAV_BUTTON_H)) {
            if (historyIndex_ > 0) {
                historyIndex_--;
                AI_LOG("Navigated to previous move, index: " + std::to_string(historyIndex_) + ", history size: " + std::to_string(moveHistory_.size()));
                renderChessboard();
                renderPieces(moveHistory_[historyIndex_]);
                SDL_RenderPresent(renderer_);
            }
        } else if (isPointInButton(x, y, 300 + 2 * (NAV_BUTTON_W + NAV_BUTTON_SPACING), NAV_BUTTON_Y, NAV_BUTTON_W, NAV_BUTTON_H)) {
            if (historyIndex_ < static_cast<int>(moveHistory_.size()) - 1) {
                historyIndex_++;
                AI_LOG("Navigated to next move, index: " + std::to_string(historyIndex_) + ", history size: " + std::to_string(moveHistory_.size()));
                renderChessboard();
                renderPieces(moveHistory_[historyIndex_]);
                SDL_RenderPresent(renderer_);
            }
        } else if (isPointInButton(x, y, 300 + 3 * (NAV_BUTTON_W + NAV_BUTTON_SPACING), NAV_BUTTON_Y, NAV_BUTTON_W, NAV_BUTTON_H)) {
            historyIndex_ = moveHistory_.size() - 1;
            AI_LOG("Navigated to last move, index: " + std::to_string(historyIndex_) + ", history size: " + std::to_string(moveHistory_.size()));
            renderChessboard();
            renderPieces(moveHistory_[historyIndex_]);
            SDL_RenderPresent(renderer_);
        } else if (!isAIActive_ || isWhiteTurn) {
            int square = getSquareFromCoords(x, y);
            if (square != -1 && board.getPieces()[square] && board.getPieces()[square]->getColor() == Color::White) {
                setSelectedSquare(square);
                isDragging_ = true;
                dragX_ = x;
                dragY_ = y;
                updateCursor(true);
                AI_LOG("Dragging started from square " + std::to_string(square));
            }
        }
    } else if (event.type == SDL_EVENT_MOUSE_MOTION && isDragging_) {
        dragX_ = event.motion.x;
        dragY_ = event.motion.y;
        updateCursor(true);
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
        if (isDragging_) {
            isDragging_ = false;
            updateCursor(false);
            int toSquare = getSquareFromCoords(event.button.x, event.button.y);
            if (selectedSquare_ != -1 && toSquare != -1 && board.getPieces()[selectedSquare_] && board.getPieces()[selectedSquare_]->getColor() == Color::White) {
                Bitboard validMoves = board.getPieces()[selectedSquare_]->generateMoves(board, selectedSquare_);
                if (validMoves.testBit(toSquare)) {
                    if (board.movePiece(selectedSquare_, toSquare)) {
                        AI_LOG("Move from " + std::to_string(selectedSquare_) + " to " + std::to_string(toSquare) + " successful");
                        isWhiteTurn = !isWhiteTurn;
                        moveHistory_.push_back(board);
                        historyIndex_ = moveHistory_.size() - 1;
                        justAIMoved_ = false;
                        if (isAIActive_ && !isWhiteTurn && !aiThreadRunning_) {
                            searchResultValid_ = false;
                            AI_LOG("Triggering AI search after player move");
                            updateSearchResult(board, isWhiteTurn);
                        }
                    } else {
                        AI_LOG("Move from " + std::to_string(selectedSquare_) + " to " + std::to_string(toSquare) + " failed: movePiece returned false");
                    }
                } else {
                    AI_LOG("Move from " + std::to_string(selectedSquare_) + " to " + std::to_string(toSquare) + " not in valid moves, bitboard: " + std::to_string(validMoves.getValue()));
                }
            }
            setSelectedSquare(-1);
        }
    }
}
