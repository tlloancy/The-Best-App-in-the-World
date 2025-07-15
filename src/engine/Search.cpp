#include "../../include/engine/Search.hpp"
#include "../../include/utils/Convert.hpp"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <chrono>

#define AI_DEBUG
#ifdef AI_DEBUG
#define AI_LOG(msg) std::cerr << "[AI_DEBUG] " << msg << std::endl
#else
#define AI_LOG(msg)
#endif

StockfishSearch::StockfishSearch(int skillLevel) : skillLevel_(skillLevel) {
    AI_LOG("Initializing StockfishSearch with skill level " + std::to_string(skillLevel_));
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
        setvbuf(writePipe_, nullptr, _IONBF, 0);
        setvbuf(readPipe_, nullptr, _IONBF, 0);
        auto start = std::chrono::steady_clock::now();
        fprintf(writePipe_, "uci\n");
        fprintf(writePipe_, "setoption name Skill Level value %d\n", skillLevel_);
        fprintf(writePipe_, "isready\n");
        fflush(writePipe_);
        char buf[256];
        int readAttempts = 0;
        while (readAttempts++ < 200) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(fileno(readPipe_), &readfds);
            struct timeval timeout = {0, 100000};
            int ready = select(fileno(readPipe_) + 1, &readfds, nullptr, nullptr, &timeout);
            if (ready > 0 && fgets(buf, sizeof(buf), readPipe_)) {
                AI_LOG("Stockfish init output: " + std::string(buf));
                if (strstr(buf, "readyok")) break;
            } else if (ready == 0) {
                AI_LOG("Stockfish init read timeout after " + std::to_string(readAttempts) + " attempts");
            } else {
                AI_LOG("Stockfish init read error: " + std::string(strerror(errno)));
                break;
            }
        }
        if (readAttempts >= 200) {
            AI_LOG("Stockfish initialization timed out after " + std::to_string(readAttempts) + " attempts, resetting pipes");
            fclose(writePipe_);
            fclose(readPipe_);
            writePipe_ = nullptr;
            readPipe_ = nullptr;
            kill(childPid_, SIGTERM);
            waitpid(childPid_, nullptr, 0);
            childPid_ = -1;
            return;
        }
        AI_LOG("Stockfish initialized with skill level " + std::to_string(skillLevel_) + " in " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()) + " ms");
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
    char buf[256];
    std::string bestMove;
    int readAttempts = 0;
    auto start = std::chrono::steady_clock::now();
    auto maxDuration = std::chrono::milliseconds(5000);
    while (readAttempts++ < 200) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start) > maxDuration) {
            AI_LOG("Stockfish move retrieval exceeded timeout, resetting pipes");
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
            return "";
        }
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fileno(readPipe_), &readfds);
        struct timeval timeout = {0, 100000};
        int ready = select(fileno(readPipe_) + 1, &readfds, nullptr, nullptr, &timeout);
        if (ready > 0 && fgets(buf, sizeof(buf), readPipe_)) {
            AI_LOG("Stockfish output: " + std::string(buf));
            if (strncmp(buf, "bestmove ", 9) == 0) {
                bestMove = std::string(buf + 9);
                bestMove = bestMove.substr(0, bestMove.find(' '));
                AI_LOG("Best move received: " + bestMove);
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
        return "";
    }
    if (bestMove.empty()) {
        AI_LOG("No valid move received from Stockfish");
    }
    AI_LOG("Stockfish move retrieval took " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()) + " ms");
    return bestMove;
}

SearchResult StockfishSearch::search(const Board& board, int depth, std::string* uciMove) {
    AI_LOG("Starting StockfishSearch::search with depth " + std::to_string(depth) + " for FEN: " + board.getFEN());
    SearchResult result;
    result.score = 0.0f;
    result.bestMove = {0, 0};
    result.topMoves.clear();

    std::string uciMoveStr = getBestMoveFromStockfish(board);
    if (!uciMoveStr.empty()) {
        if (uciMove) *uciMove = uciMoveStr;
        result.bestMove = fromUCI(uciMoveStr);
        AI_LOG("UCI move: " + uciMoveStr + ", converted to from=" + std::to_string(result.bestMove.first) + ", to=" + std::to_string(result.bestMove.second));
        if (result.bestMove.first >= 64 || result.bestMove.second >= 64 || result.bestMove.first < 0 || result.bestMove.second < 0) {
            AI_LOG("Invalid move from Stockfish UCI: " + uciMoveStr);
            result.score = 0.0f;
            result.bestMove = {0, 0};
            return result;
        }
        auto legalMoves = board.getLegalMoves(Color::Black);
        bool isLegal = false;
        for (const auto& move : legalMoves) {
            if (move.first == result.bestMove.first && move.second == result.bestMove.second) {
                isLegal = true;
                break;
            }
        }
        if (!isLegal) {
            AI_LOG("Stockfish move " + uciMoveStr + " is not legal for black");
            result.score = 0.0f;
            result.bestMove = {0, 0};
            return result;
        }
        Board tempBoard(board);
        AI_LOG("Board before move: " + tempBoard.getFEN());
        if (!tempBoard.movePiece(result.bestMove.first, result.bestMove.second)) {
            AI_LOG("Stockfish move " + uciMoveStr + " failed to apply");
            result.score = 0.0f;
            result.bestMove = {0, 0};
            return result;
        }
        AI_LOG("Board after move: " + tempBoard.getFEN());
        result.score = 0.0f;
        result.topMoves = {result.bestMove};
        AI_LOG("StockfishSearch completed, move: " + uciMoveStr + ", score: " + std::to_string(result.score));
    } else {
        AI_LOG("No move from Stockfish");
        result.score = 0.0f;
        result.bestMove = {0, 0};
    }
    return result;
}
