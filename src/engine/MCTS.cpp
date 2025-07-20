#include "../../include/engine/MCTS.hpp"
#include "../../include/core/Evaluator.hpp"
#include "../../include/utils/Convert.hpp"
#include <random>
#include <cmath>
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

struct MCTSNode {
    char pad[8];
    float totalScore = 0.0f;
    int visits = 0;
    Board board;
    std::pair<int, int> move;
    std::vector<MCTSNode*> children;
    MCTSNode* parent = nullptr;

    MCTSNode(const Board& b, const std::pair<int, int>& m = {0, 0}) : board(b), move(m) {}
    ~MCTSNode() { for (auto child : children) delete child; }
};

extern "C" float _mcts_uct_score(const MCTSNode* node, int parentVisits);

SearchResult MCTS::evaluate(const Board& board, int iterations) {
    AI_LOG("Starting MCTS evaluation with " + std::to_string(iterations) + " iterations for board FEN: " + board.getFEN());
    SearchResult result;
    result.score = 0.0f;
    result.bestMove = {0, 0};
    result.topMoves.clear();

    MCTSNode* root = new MCTSNode(board);
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < iterations; ++i) {
        MCTSNode* node = root;
        Board currentBoard = board;
        std::vector<MCTSNode*> path = {node};

        // Selection
        while (!node->children.empty()) {
            node = *std::max_element(node->children.begin(), node->children.end(),
                [node](const MCTSNode* a, const MCTSNode* b) { return _mcts_uct_score(a, node->visits) < _mcts_uct_score(b, node->visits); });
            if (!currentBoard.movePiece(node->move.first, node->move.second)) {
                AI_LOG("Failed to apply move from " + std::to_string(node->move.first) + " to " + std::to_string(node->move.second));
                break;
            }
            path.push_back(node);
        }

        // Expansion
        if (node->visits > 0 && !node->board.isCheckmate() && !node->board.isStalemate() && !node->board.isDraw()) {
            auto moves = node->board.getLegalMoves(node->board.isWhiteToMove() ? Color::White : Color::Black);
            AI_LOG("Expanding node with " + std::to_string(moves.size()) + " legal moves");
            for (const auto& move : moves) {
                Board temp(node->board);
                if (!temp.movePiece(move.first, move.second)) {
                    AI_LOG("Failed to apply move from " + std::to_string(move.first) + " to " + std::to_string(move.second));
                    continue;
                }
                MCTSNode* child = new MCTSNode(temp, move);
                child->parent = node;
                node->children.push_back(child);
            }
            if (!node->children.empty()) {
                std::uniform_int_distribution<size_t> dist(0, node->children.size() - 1);
                node = node->children[dist(gen)];
                if (!currentBoard.movePiece(node->move.first, node->move.second)) {
                    AI_LOG("Failed to apply expansion move from " + std::to_string(node->move.first) + " to " + std::to_string(node->move.second));
                    continue;
                }
                path.push_back(node);
            }
        }

        // Simulation
        float simScore = 0.0f;
        int simDepth = 0;
        while (simDepth < 10 && !currentBoard.isCheckmate() && !currentBoard.isStalemate() && !currentBoard.isDraw()) {
            auto moves = currentBoard.getLegalMoves(currentBoard.isWhiteToMove() ? Color::White : Color::Black);
            if (moves.empty()) {
                AI_LOG("No legal moves in simulation at depth " + std::to_string(simDepth));
                break;
            }
            std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
            auto move = moves[dist(gen)];
            if (!currentBoard.movePiece(move.first, move.second)) {
                AI_LOG("Failed to apply simulation move from " + std::to_string(move.first) + " to " + std::to_string(move.second));
                break;
            }
            simDepth++;
        }
        Evaluator evaluator;
        simScore = evaluator.evaluate(currentBoard);
        if (std::isnan(simScore) || std::isinf(simScore)) {
            AI_LOG("Invalid simulation score: " + std::to_string(simScore));
            simScore = 0.0f;
        }
        AI_LOG("Simulation completed, score: " + std::to_string(simScore));

        // Backpropagation
        for (MCTSNode* n : path) {
            n->visits++;
            n->totalScore += simScore;
            simScore = -simScore;
        }
    }

    if (!root->children.empty()) {
        auto bestChild = *std::max_element(root->children.begin(), root->children.end(),
            [](const MCTSNode* a, const MCTSNode* b) { return a->totalScore / a->visits < b->totalScore / b->visits; });
        result.score = bestChild->totalScore / bestChild->visits;
        if (std::isnan(result.score) || std::isinf(result.score)) {
            AI_LOG("Invalid final score: " + std::to_string(result.score));
            result.score = 0.0f;
        }
        result.bestMove = bestChild->move;
        std::vector<std::pair<MCTSNode*, float>> scoredChildren;
        for (auto child : root->children) {
            scoredChildren.emplace_back(child, child->visits > 0 ? child->totalScore / child->visits : 0.0f);
        }
        std::sort(scoredChildren.begin(), scoredChildren.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
        for (size_t i = 0; i < scoredChildren.size() && i < 3; ++i) {
            result.topMoves.push_back(scoredChildren[i].first->move);
        }
        AI_LOG("MCTS evaluation completed, best move: " +
               std::string(1, 'a' + (result.bestMove.first % 8)) + std::to_string(8 - (result.bestMove.first / 8)) + "-" +
               std::string(1, 'a' + (result.bestMove.second % 8)) + std::to_string(8 - (result.bestMove.second / 8)) + ", score: " + std::to_string(result.score));
    } else {
        AI_LOG("No valid moves found in MCTS evaluation");
    }

    delete root;
    return result;
}

MCTSSearch::MCTSSearch(int elo, int thinkTimeMs) : elo_(elo), thinkTimeMs_(thinkTimeMs) {
    AI_LOG("Initializing MCTSSearch with Elo " + std::to_string(elo_) + " and think time " + std::to_string(thinkTimeMs_) + "ms");
    startStockfish();
}

MCTSSearch::~MCTSSearch() {
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
    AI_LOG("MCTSSearch destructor called");
}

void MCTSSearch::startStockfish() {
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
        execl(STOCKFISH_PATH, "stockfish", (char *)nullptr);
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
        fprintf(writePipe_, "setoption name UCI_LimitStrength value %s\n", elo_ < 3190 ? "true" : "false");
        fprintf(writePipe_, "setoption name UCI_Elo value %d\n", std::min(elo_, 3190));
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
                if (strstr(buf, "uciok")) break;
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
        fprintf(writePipe_, "setoption name UCI_LimitStrength value %s\n", elo_ < 3190 ? "true" : "false");
        fprintf(writePipe_, "setoption name UCI_Elo value %d\n", std::min(elo_, 3190));
        fprintf(writePipe_, "isready\n");
        fflush(writePipe_);
        readAttempts = 0;
        while (readAttempts++ < 200) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(fileno(readPipe_), &readfds);
            struct timeval timeout = {0, 100000};
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
            AI_LOG("Stockfish ready check timed out after " + std::to_string(readAttempts) + " attempts, resetting pipes");
            fclose(writePipe_);
            fclose(readPipe_);
            writePipe_ = nullptr;
            readPipe_ = nullptr;
            kill(childPid_, SIGTERM);
            waitpid(childPid_, nullptr, 0);
            childPid_ = -1;
            return;
        }
        AI_LOG("Stockfish initialized with Elo " + std::to_string(elo_) + " in " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()) + " ms");
    }
}

void MCTSSearch::setSkillLevel(int elo, int thinkTimeMs) {
    elo_ = std::max(0, std::min(10000, elo));
    thinkTimeMs_ = thinkTimeMs;
    if (writePipe_) {
        fprintf(writePipe_, "setoption name UCI_LimitStrength value %s\n", elo_ < 3190 ? "true" : "false");
        fprintf(writePipe_, "setoption name UCI_Elo value %d\n", std::min(elo_, 3190));
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
            AI_LOG("Stockfish ready check timed out after " + std::to_string(readAttempts) + " attempts, resetting pipes");
            fclose(writePipe_);
            fclose(readPipe_);
            writePipe_ = nullptr;
            readPipe_ = nullptr;
            kill(childPid_, SIGTERM);
            waitpid(childPid_, nullptr, 0);
            childPid_ = -1;
            startStockfish();
        }
        AI_LOG("Stockfish skill level updated to Elo " + std::to_string(elo_) + ", think time " + std::to_string(thinkTimeMs_) + "ms");
    }
}

std::string MCTSSearch::getBestMoveFromStockfish(const Board& board) {
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
    if (thinkTimeMs_ > 0) {
        fprintf(writePipe_, "go movetime %d\n", thinkTimeMs_);
    } else {
        fprintf(writePipe_, "go depth 10\n");
    }
    fflush(writePipe_);
    char buf[256];
    std::string bestMove;
    int readAttempts = 0;
    auto start = std::chrono::steady_clock::now();
    auto maxDuration = std::chrono::milliseconds(thinkTimeMs_ > 0 ? thinkTimeMs_ + 1000 : 5000);
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

SearchResult MCTSSearch::search(const Board& board, int depth, std::string* uciMove) {
    AI_LOG("Starting MCTSSearch::search with depth " + std::to_string(depth) + " for FEN: " + board.getFEN());
    SearchResult result;
    result.score = 0.0f;
    result.bestMove = {0, 0};
    result.topMoves.clear();

    if (elo_ >= 1500 && writePipe_ && readPipe_) {
        std::string uci = getBestMoveFromStockfish(board);
        if (!uci.empty()) {
            if (uciMove) *uciMove = uci;
            std::pair<int, int> bestMove = fromUCI(uci);
            AI_LOG("UCI move: " + uci + ", converted to from=" + std::to_string(bestMove.first) + ", to=" + std::to_string(bestMove.second));
            if (bestMove.first >= 64 || bestMove.second >= 64 || bestMove.first < 0 || bestMove.second < 0) {
                AI_LOG("Invalid move from Stockfish UCI: " + uci + ", falling back to MCTS evaluation");
                MCTS mcts;
                return mcts.evaluate(board, 100);
            }
            auto legalMoves = board.getLegalMoves(Color::Black);
            bool isLegal = false;
            for (const auto& move : legalMoves) {
                if (move.first == bestMove.first && move.second == bestMove.second) {
                    isLegal = true;
                    break;
                }
            }
            if (!isLegal) {
                AI_LOG("Stockfish move " + uci + " is not legal for black, falling back to MCTS evaluation");
                MCTS mcts;
                return mcts.evaluate(board, 100);
            }
            Board tempBoard(board);
            AI_LOG("Board before move: " + tempBoard.getFEN());
            if (!tempBoard.movePiece(bestMove.first, bestMove.second)) {
                AI_LOG("Stockfish move " + uci + " failed to apply, falling back to MCTS evaluation");
                MCTS mcts;
                return mcts.evaluate(board, 100);
            }
            AI_LOG("Board after move: " + tempBoard.getFEN());
            Evaluator evaluator;
            float score = evaluator.evaluate(tempBoard);
            if (std::isnan(score) || std::isinf(score)) {
                AI_LOG("Invalid Stockfish evaluation score: " + std::to_string(score));
                score = 0.0f;
            }
            result.score = score;
            result.bestMove = bestMove;
            result.topMoves = {bestMove};
            AI_LOG("MCTSSearch completed, move: " + uci + ", score: " + std::to_string(score));
        } else {
            AI_LOG("No move from Stockfish, falling back to MCTS evaluation");
            MCTS mcts;
            return mcts.evaluate(board, 100);
        }
    } else {
        MCTS mcts;
        result = mcts.evaluate(board, 100);
        if (uciMove && result.bestMove.first != 0 && result.bestMove.second != 0) {
            *uciMove = std::string(1, 'a' + (result.bestMove.first % 8)) + std::to_string(8 - (result.bestMove.first / 8)) +
                       std::string(1, 'a' + (result.bestMove.second % 8)) + std::to_string(8 - (result.bestMove.second / 8));
            AI_LOG("MCTS move: " + *uciMove + ", score: " + std::to_string(result.score));
        }
    }
    return result;
}
