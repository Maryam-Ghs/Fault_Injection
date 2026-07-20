/* LLM input variant 8: sparse-skewed */
// AlphaGo algorithm mock‑up – version #7
// Simplified Monte‑Carlo Tree Search for a 3×3 board (tic‑tac‑toe like)
// Uses only int and float, heap allocation, manual loop unrolling and temporaries
// Everything is inside main() – no external input

#include <iostream>
#include <cstdlib>

int main()
{
    // ----- board setup (deterministic test vector) -----
    int boardSize = 3;
    int totalCells = boardSize * boardSize;          // 9
    int* board = new int[totalCells];                // 0 = empty, 1 = black, -1 = white

    // sparse, imbalanced, clustered input:
    //  1  0   0
    //  0 -1   0
    //  0  0   0
    board[0] =  1;   board[1] =  0;  board[2] =  0;
    board[3] =  0;   board[4] = -1;  board[5] =  0;
    board[6] =  0;   board[7] =  0;  board[8] =  0;

    // ----- MCTS data structures (heap) -----
    int* visitCount = new int[totalCells];
    float* totalValue = new float[totalCells];

    // initialise
    for (int i = 0; i < totalCells; ++i) {
        visitCount[i] = 0;
        totalValue[i] = 0.0f;
    }

    // ----- simulation parameters -----
    int simulations = 16;                     // deterministic small number
    int player = 1;                           // black to move

    // ----- simple policy: first empty cell (deterministic) -----
    for (int sim = 0; sim < simulations; ++sim) {
        // copy board for this simulation
        int* simBoard = new int[totalCells];
        for (int i = 0; i < totalCells; ++i) {
            simBoard[i] = board[i];
        }

        // ----- selection & expansion (first empty) -----
        int selectedMove = -1;
        // manual unrolled search for empty cell
        if (simBoard[0] == 0) selectedMove = 0;
        else if (simBoard[1] == 0) selectedMove = 1;
        else if (simBoard[2] == 0) selectedMove = 2;
        else if (simBoard[3] == 0) selectedMove = 3;
        else if (simBoard[4] == 0) selectedMove = 4;
        else if (simBoard[5] == 0) selectedMove = 5;
        else if (simBoard[6] == 0) selectedMove = 6;
        else if (simBoard[7] == 0) selectedMove = 7;
        else if (simBoard[8] == 0) selectedMove = 8;

        // if no move possible, break
        if (selectedMove == -1) {
            delete[] simBoard;
            break;
        }

        // place the stone
        simBoard[selectedMove] = player;

        // ----- evaluation (simple heuristic) -----
        // value = (black stones – white stones) / totalCells   (float)
        int blackCount = 0;
        int whiteCount = 0;

        // unrolled counting
        if (simBoard[0] ==  1) ++blackCount; else if (simBoard[0] == -1) ++whiteCount;
        if (simBoard[1] ==  1) ++blackCount; else if (simBoard[1] == -1) ++whiteCount;
        if (simBoard[2] ==  1) ++blackCount; else if (simBoard[2] == -1) ++whiteCount;
        if (simBoard[3] ==  1) ++blackCount; else if (simBoard[3] == -1) ++whiteCount;
        if (simBoard[4] ==  1) ++blackCount; else if (simBoard[4] == -1) ++whiteCount;
        if (simBoard[5] ==  1) ++blackCount; else if (simBoard[5] == -1) ++whiteCount;
        if (simBoard[6] ==  1) ++blackCount; else if (simBoard[6] == -1) ++whiteCount;
        if (simBoard[7] ==  1) ++blackCount; else if (simBoard[7] == -1) ++whiteCount;
        if (simBoard[8] ==  1) ++blackCount; else if (simBoard[8] == -1) ++whiteCount;

        int diff = blackCount - whiteCount;                // temporary variable
        float leafValue = 0.0f;
        leafValue = static_cast<float>(diff) / static_cast<float>(totalCells);

        // ----- back‑propagation -----
        // update statistics for the selected move only (since tree is flat)
        int prevVisits = visitCount[selectedMove];
        float prevTotal = totalValue[selectedMove];

        // split into temporaries
        int newVisits = prevVisits + 1;
        float newTotal = prevTotal + leafValue;

        visitCount[selectedMove] = newVisits;
        totalValue[selectedMove] = newTotal;

        // clean up simulation board
        delete[] simBoard;
    }

    // ----- choose best move (highest visit count) -----
    int bestMove = -1;
    int bestVisits = -1;
    // unrolled comparison
    if (visitCount[0] > bestVisits) { bestVisits = visitCount[0]; bestMove = 0; }
    if (visitCount[1] > bestVisits) { bestVisits = visitCount[1]; bestMove = 1; }
    if (visitCount[2] > bestVisits) { bestVisits = visitCount[2]; bestMove = 2; }
    if (visitCount[3] > bestVisits) { bestVisits = visitCount[3]; bestMove = 3; }
    if (visitCount[4] > bestVisits) { bestVisits = visitCount[4]; bestMove = 4; }
    if (visitCount[5] > bestVisits) { bestVisits = visitCount[5]; bestMove = 5; }
    if (visitCount[6] > bestVisits) { bestVisits = visitCount[6]; bestMove = 6; }
    if (visitCount[7] > bestVisits) { bestVisits = visitCount[7]; bestMove = 7; }
    if (visitCount[8] > bestVisits) { bestVisits = visitCount[8]; bestMove = 8; }

    // ----- output -----
    std::cout << "AlphaGo mock‑up (version #7) result:\n";
    std::cout << "Best move index: " << bestMove << " (row " << bestMove / boardSize
              << ", col " << bestMove % boardSize << ")\n";
    std::cout << "Visit count: " << bestVisits << "\n";

    // optional: show final board after applying best move
    if (bestMove != -1) {
        board[bestMove] = player;
    }
    std::cout << "Resulting board:\n";
    for (int r = 0; r < boardSize; ++r) {
        for (int c = 0; c < boardSize; ++c) {
            int idx = r * boardSize + c;
            int v = board[idx];
            if (v ==  1) std::cout << " X";
            else if (v == -1) std::cout << " O";
            else std::cout << " .";
        }
        std::cout << "\n";
    }

    // ----- clean up -----
    delete[] board;
    delete[] visitCount;
    delete[] totalValue;

    return 0;
}
