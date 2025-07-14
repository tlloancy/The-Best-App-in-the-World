#include "../../include/engine/Search.hpp"
#include "../../include/utils/Convert.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <chrono>

// Define AI_DEBUG to enable AI-specific debug output (comment out to disable)
#define AI_DEBUG

#ifdef AI_DEBUG
#define AI_LOG(msg) std::cerr << "[AI_DEBUG] " << msg << std::endl
#else
#define AI_LOG(msg)
#endif

StockfishSearch::StockfishSearch(int skillLevel) : skillLevel_(skillLevel) {
    AI_LOG("Initializing StockfishSearch with skill level " + std::to_string(skillLevel));
    startStockfish();
}

StockfishSearch::~StockfishSearch() {
    if (writePipe_) {
        fprintf(writePipe_, "quit\n");
        fflush(writePipe_);
        fclose(writePipe_);
        writePipe_ = nullptr;
    }
    if (readPipe_) {
        fclose(readPipe_);
        readPipe_ = nullptr;
    }
    if (childPid_ > 0) {
        kill(childPid_, SIGTERM);
        waitpid(childPid_, nullptr, 0);
        childPid_ = -1;
        AI_LOG("Stockfish process terminated, pid: " + std::to_string(childPid_));
    }
    AI_LOG("StockfishSearch destructor called");
}

void StockfishSearch::startStockfish() {
    if (childPid_ > 0) {
        AI_LOG("Existing Stockfish process found, pid: " + std::to_string(childPid_) + ", terminating");
        kill(childPid_, SIGTERM);
        waitpid(childPid_, nullptr, 0);
        childPid_ = -1;
    }
    int toChild[2];
    int fromChild[2];
    if (pipe(toChild) == -1 || pipe(fromChild) == -1) {
        AI_LOG("Failed to create pipes: " + std::string(strerror(errno)));
        return;
    }

    childPid_ = fork();
    if (childPid_ == -1) {
        AI_LOG("Failed to fork: " + std::string(strerror(errno)));
        close(toChild[0]);
        close(toChild[1]);
        close(fromChild[0]);
        close(fromChild[1]);
        return;
    }

    if (childPid_ == 0) {
        // Child process
        close(toChild[1]);
        close(fromChild[0]);
        dup2(toChild[0], STDIN_FILENO);
        dup2(fromChild[1], STDOUT_FILENO);
        close(toChild[0]);
        close(fromChild[1]);
        execl("/mnt/c/Users/lloan/Documents/cosmic_chess/build/stockfish", "stockfish", (char *)nullptr);
        AI_LOG("Failed to exec stockfish: " + std::string(strerror(errno)));
        exit(1);
    } else {
        // Parent process
        close(toChild[0]);
        close(fromChild[1]);
        writePipe_ = fdopen(toChild[1], "w");
        readPipe_ = fdopen(fromChild[0], "r");
        if (!writePipe_ || !readPipe_) {
            AI_LOG("Failed to open pipes as FILE*: " + std::string(strerror(errno)));
            if (writePipe_) fclose(writePipe_);
            if (readPipe_) fclose(readPipe_);
            if (childPid_ > 0) {
                kill(childPid_, SIGTERM);
                waitpid(childPid_, nullptr, 0);
            }
            return;
        }
        setvbuf(writePipe_, nullptr, _IONBF, 0); // Disable buffering
        setvbuf(readPipe_, nullptr, _IONBF, 0); // Disable buffering
        auto start = std::chrono::steady_clock::now();
        fprintf(writePipe_, "uci\n");
        fflush(writePipe_);
        char buf[256];
        int readAttempts = 0;
        while (readAttempts++ < 200) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(fileno(readPipe_), &readfds);
            struct timeval timeout = {0, 100000}; // 100ms timeout
            int ready = select(fileno(readPipe_) + 1, &readfds, nullptr, nullptr, &timeout);
            if (ready > 0 && fgets(buf, sizeof(buf), readPipe_)) {
                AI_LOG("Stockfish init output: " + std::string(buf));
                if (strstr(buf, "uciok")) break;
            } else if (ready == 0) {
                AI_LOG("Stockfish init read timeout after " + std::to_string(readAttempts) + " attempts");
            } else {
                AI_LOG("Stockfish init read error: " + std::string(strerror(errno)));
                break;
            }
        }
        if (readAttempts >= 200) {
            AI_LOG("Stockfish initialization timed out after " + std::to_string(readAttempts) + " attempts");
            fclose(writePipe_);
            fclose(readPipe_);
            writePipe_ = nullptr;
            readPipe_ = nullptr;
            kill(childPid_, SIGTERM);
            waitpid(childPid_, nullptr, 0);
            childPid_ = -1;
            return;
        }
        fprintf(writePipe_, "setoption name Skill Level value %d\n", skillLevel_);
        fprintf(writePipe_, "isready\n");
        fflush(writePipe_);
        readAttempts = 0;
        while (readAttempts++ < 200) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(fileno(readPipe_), &readfds);
            struct timeval timeout = {0, 100000}; // 100ms timeout
            int ready = select(fileno(readPipe_) + 1, &readfds, nullptr, nullptr, &timeout);
            if (ready > 0 && fgets(buf, sizeof(buf), readPipe_)) {
                AI_LOG("Stockfish isready output: " + std::string(buf));
                if (strstr(buf, "readyok")) break;
            } else if (ready == 0) {
                AI_LOG("Stockfish isready read timeout after " + std::to_string(readAttempts) + " attempts");
            } else {
                AI_LOG("Stockfish isready read error: " + std::string(strerror(errno)));
                break;
            }
        }
        if (readAttempts >= 200) {
            AI_LOG("Stockfish isready timed out after " + std::to_string(readAttempts) + " attempts");
            fclose(writePipe_);
            fclose(readPipe_);
            writePipe_ = nullptr;
            readPipe_ = nullptr;
            kill(childPid_, SIGTERM);
            waitpid(childPid_, nullptr, 0);
            childPid_ = -1;
            return;
        }
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        AI_LOG("Stockfish initialized with skill level " + std::to_string(skillLevel_) + " in " + std::to_string(duration) + " ms");
    }
}

std::string StockfishSearch::getBestMoveFromStockfish(const Board& board) {
    if (!writePipe_ || !readPipe_) {
        AI_LOG("Stockfish pipes not open, resetting");
        startStockfish();
        if (!writePipe_ || !readPipe_) {
            AI_LOG("Failed to reset Stockfish pipes");
            return "";
        }
    }
    std::string fen = board.getFEN();
    AI_LOG("Sending FEN to Stockfish: " + fen);
    fprintf(writePipe_, "position fen %s\n", fen.c_str());
    fprintf(writePipe_, "go depth 10\n");
    fflush(writePipe_);
    auto start = std::chrono::steady_clock::now();
    char buf[256];
    std::string bestMove;
    int readAttempts = 0;
    auto maxDuration = std::chrono::milliseconds(5000); // 5s timeout
    while (readAttempts++ < 200) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start) > maxDuration) {
            AI_LOG("Stockfish move retrieval exceeded 5s timeout, resetting pipes");
            fclose(writePipe_);
            fclose(readPipe_);
            writePipe_ = nullptr;
            readPipe_ = nullptr;
            if (childPid_ > 0) {
                kill(childPid_, SIGTERM);
                waitpid(childPid_, nullptr, 0);
                childPid_ = -1;
            }
            startStockfish();
            break;
        }
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fileno(readPipe_), &readfds);
        struct timeval timeout = {0, 100000}; // 100ms timeout
        int ready = select(fileno(readPipe_) + 1, &readfds, nullptr, nullptr, &timeout);
        if (ready > 0 && fgets(buf, sizeof(buf), readPipe_)) {
            AI_LOG("Stockfish output: " + std::string(buf));
            if (strstr(buf, "bestmove")) {
                char* moveStr = strstr(buf, "bestmove ");
                if (moveStr) {
                    moveStr += 9;
                    char* end = strchr(moveStr, ' ');
                    if (end) *end = '\0';
                    bestMove = moveStr;
                    AI_LOG("Best move received: " + bestMove);
                } else {
                    AI_LOG("No valid move in bestmove line: " + std::string(buf));
                }
                break;
            }
        } else if (ready == 0) {
            AI_LOG("Stockfish move read timeout after " + std::to_string(readAttempts) + " attempts");
        } else {
            AI_LOG("Stockfish move read error: " + std::string(strerror(errno)));
            break;
        }
    }
    if (readAttempts >= 200) {
        AI_LOG("Stockfish move retrieval timed out after " + std::to_string(readAttempts) + " attempts, resetting pipes");
        fclose(writePipe_);
        fclose(readPipe_);
        writePipe_ = nullptr;
        readPipe_ = nullptr;
        if (childPid_ > 0) {
            kill(childPid_, SIGTERM);
            waitpid(childPid_, nullptr, 0);
            childPid_ = -1;
        }
        startStockfish();
    }
    if (bestMove.empty()) {
        AI_LOG("No valid move received from Stockfish");
    }
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    AI_LOG("Stockfish move retrieval took " + std::to_string(duration) + " ms");
    return bestMove;
}

SearchResult StockfishSearch::search(const Board& board, int depth) {
    AI_LOG("Starting StockfishSearch::search with depth " + std::to_string(depth) + " for FEN: " + board.getFEN());
    std::string uci = getBestMoveFromStockfish(board);
    if (uci.empty()) {
        AI_LOG("No move from Stockfish, returning default result");
        return {0.0f, {0, 0}, {}};
    }
    Move bestMove = Convert::fromUCI(uci);
    AI_LOG("UCI move: " + uci + ", converted to from=" + std::to_string(bestMove.from) + ", to=" + std::to_string(bestMove.to));
    if (bestMove.from >= 64 || bestMove.to >= 64 || bestMove.from < 0 || bestMove.to < 0) {
        AI_LOG("Invalid move from Stockfish UCI: " + uci + ", returning default result");
        return {0.0f, {0, 0}, {}};
    }
    // Verify move is legal
    auto legalMoves = board.getLegalMoves(board.isWhiteToMove() ? Color::White : Color::Black);
    bool isLegal = false;
    for (const auto& move : legalMoves) {
        if (move.first == bestMove.from && move.second == bestMove.to) {
            isLegal = true;
            break;
        }
    }
    if (!isLegal) {
        AI_LOG("Stockfish move " + uci + " is not legal, returning default result");
        return {0.0f, {0, 0}, {}};
    }
    Board tempBoard(board);
    AI_LOG("Board before move: " + tempBoard.getFEN());
    if (!tempBoard.movePiece(bestMove.from, bestMove.to)) {
        AI_LOG("Stockfish move " + uci + " failed to apply, returning default result");
        return {0.0f, {0, 0}, {}};
    }
    AI_LOG("Board after move: " + tempBoard.getFEN());
    float score = 0.0f; // Simplified, as Stockfish doesn't return a score directly
    AI_LOG("StockfishSearch completed, move: " + uci + ", score: " + std::to_string(score));
    return {score, bestMove, {bestMove}};
}
