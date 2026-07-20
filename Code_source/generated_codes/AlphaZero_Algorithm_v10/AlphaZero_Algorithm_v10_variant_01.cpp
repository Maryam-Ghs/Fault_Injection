/* LLM input variant 1: minimal-boundary */
// AlphaZero simplified demo – version #10
// -------------------------------------------------
// This program runs a deterministic Monte‑Carlo Tree Search
// on a tiny Tic‑Tac‑Toe board using only int and float types.
// All data are kept in stack arrays and helper functions.
// -------------------------------------------------

#include <iostream>
#include <cstdio>

// ------------------------------------------------------------------
// Simple deterministic pseudo‑random generator (LCG)
// ------------------------------------------------------------------
int g_seed = 987654321;                     // global seed (int only)

int rng_next()
{
    // Linear congruential formula: X_{n+1} = (a * X_n + c) mod 2^31
    g_seed = (g_seed * 1103515245 + 12345) & 0x7fffffff;
    return g_seed;
}

// ------------------------------------------------------------------
// Board utilities (3x3 Tic‑Tac‑Toe)
// ------------------------------------------------------------------
void board_clear(int board[9])
{
    int i = 0;
    while (i < 9)
    {
        board[i] = 0;            // 0 = empty, 1 = X, -1 = O
        i = i + 1;
    }
}

void board_copy(int src[9], int dst[9])
{
    int i = 0;
    while (i < 9)
    {
        dst[i] = src[i];
        i = i + 1;
    }
}

bool board_full(const int board[9])
{
    int i = 0;
    while (i < 9)
    {
        if (board[i] == 0) return false;
        i = i + 1;
    }
    return true;
}

// Return true if player (1 or -1) has three in a row
bool board_win(const int board[9], int player)
{
    // rows
    int r = 0;
    while (r < 3)
    {
        int idx = r * 3;
        if (board[idx] == player && board[idx + 1] == player && board[idx + 2] == player) return true;
        r = r + 1;
    }
    // columns
    int c = 0;
    while (c < 3)
    {
        if (board[c] == player && board[c + 3] == player && board[c + 6] == player) return true;
        c = c + 1;
    }
    // diagonals
    if (board[0] == player && board[4] == player && board[8] == player) return true;
    if (board[2] == player && board[4] == player && board[6] == player) return true;
    return false;
}

// Collect indices of empty squares
void board_legal_moves(const int board[9], int moves[9], int &moveCnt)
{
    moveCnt = 0;
    int i = 0;
    while (i < 9)
    {
        if (board[i] == 0)
        {
            moves[moveCnt] = i;
            moveCnt = moveCnt + 1;
        }
        i = i + 1;
    }
}

// Apply a move (mutates board)
void board_apply(int board[9], int pos, int player)
{
    board[pos] = player;
}

// ------------------------------------------------------------------
// Simple evaluation function – returns a float value from -1..1
// ------------------------------------------------------------------
float eval_board(const int board[9], int player)
{
    // Very naive: +1 if player wins, -1 if opponent wins, 0 otherwise
    if (board_win(board, player)) return 1.0f;
    if (board_win(board, -player)) return -1.0f;
    return 0.0f;
}

// ------------------------------------------------------------------
// Node structure for MCTS (stack‑allocated)
// ------------------------------------------------------------------
struct Node
{
    int board[9];          // current board
    int player;            // player to move at this node
    int visits;            // N(s)
    float valueSum;        // W(s) – sum of values
    int childIdx[9];       // indices of child nodes in global pool
    int childCnt;          // number of children
    int moveFromParent;    // move that led from parent to this node
};

// ------------------------------------------------------------------
// Global pool of nodes (fixed size, on stack)
// ------------------------------------------------------------------
#define MAX_NODES 1024
Node g_nodes[MAX_NODES];
int g_nextNode = 0;

// ------------------------------------------------------------------
// Create a new node, returns its index (or -1 if pool exhausted)
// ------------------------------------------------------------------
int node_create(const int board[9], int player, int moveFromParent)
{
    if (g_nextNode >= MAX_NODES) return -1;
    int id = g_nextNode;
    g_nextNode = g_nextNode + 1;

    // copy board
    int i = 0;
    while (i < 9)
    {
        g_nodes[id].board[i] = board[i];
        i = i + 1;
    }
    g_nodes[id].player       = player;
    g_nodes[id].visits       = 0;
    g_nodes[id].valueSum     = 0.0f;
    g_nodes[id].childCnt     = 0;
    g_nodes[id].moveFromParent = moveFromParent;

    // initialise child list to -1
    int j = 0;
    while (j < 9)
    {
        g_nodes[id].childIdx[j] = -1;
        j = j + 1;
    }
    return id;
}

// ------------------------------------------------------------------
// Expand a node: generate all legal moves as children
// ------------------------------------------------------------------
void node_expand(int nodeId)
{
    Node &node = g_nodes[nodeId];
    int moves[9];
    int moveCnt = 0;
    board_legal_moves(node.board, moves, moveCnt);

    int k = 0;
    while (k < moveCnt)
    {
        int newBoard[9];
        board_copy(node.board, newBoard);
        board_apply(newBoard, moves[k], node.player);

        int childId = node_create(newBoard, -node.player, moves[k]);
        node.childIdx[node.childCnt] = childId;
        node.childCnt = node.childCnt + 1;
        k = k + 1;
    }
}

// ------------------------------------------------------------------
// UCT selection – return child index (in global pool)
// ------------------------------------------------------------------
int node_select_uct(int nodeId)
{
    Node &node = g_nodes[nodeId];
    int bestChild = -1;
    float bestScore = -1.0e30f;          // very low sentinel

    int i = 0;
    while (i < node.childCnt)
    {
        int childId = node.childIdx[i];
        Node &child = g_nodes[childId];

        // Compute exploitation term: Q = W/N
        float q = 0.0f;
        if (child.visits > 0)
        {
            float w = child.valueSum;
            int   n = child.visits;
            q = w / (float)n;
        }

        // Compute exploration term: U = c * sqrt(ln(N_parent)/N_child)
        float u = 0.0f;
        if (child.visits > 0)
        {
            float logParent = 0.0f;
            int   nParent   = node.visits;
            // expand log manually (approximation using series)
            // log(x) ≈ 2 * ((x-1)/(x+1) + 1/3*((x-1)/(x+1))^3 )
            float ratio = ((float)nParent - 1.0f) / ((float)nParent + 1.0f);
            float ratio2 = ratio * ratio;
            logParent = 2.0f * (ratio + (ratio * ratio2) / 3.0f);

            float sqrtTerm = logParent / (float)child.visits;
            // simple sqrt via Newton iterations (2 iterations)
            float s = sqrtTerm;
            int it = 0;
            while (it < 2)
            {
                if (s <= 0.0f) break;
                s = 0.5f * (s + sqrtTerm / s);
                it = it + 1;
            }
            float c = 1.414f;               // sqrt(2) as constant
            u = c * s;
        }

        float score = q + u;

        // verbose printing of each child's score
        std::printf("Select: child %d (move %d) -> Q=%.4f U=%.4f score=%.4f\n",
                    i, g_nodes[childId].moveFromParent, q, u, score);

        if (score > bestScore)
        {
            bestScore = score;
            bestChild = childId;
        }
        i = i + 1;
    }
    // If no child selected (all unvisited), pick first child
    if (bestChild == -1 && node.childCnt > 0)
        bestChild = node.childIdx[0];
    return bestChild;
}

// ------------------------------------------------------------------
// Simulation (rollout) – deterministic policy (random but seeded)
// ------------------------------------------------------------------
float rollout_simulate(const int board[9], int player)
{
    int simBoard[9];
    board_copy(board, simBoard);
    int curPlayer = player;

    // loop until terminal state
    while (true)
    {
        if (board_win(simBoard, -curPlayer))
        {
            // previous player won
            return (float)(-curPlayer);
        }
        if (board_full(simBoard))
        {
            return 0.0f;                 // draw
        }

        // deterministic "random" move selection
        int legal[9];
        int cnt = 0;
        board_legal_moves(simBoard, legal, cnt);
        int r = rng_next();
        int idx = r % cnt;               // deterministic index
        board_apply(simBoard, legal[idx], curPlayer);
        curPlayer = -curPlayer;
    }
}

// ------------------------------------------------------------------
// Back‑propagation – propagate value up the tree
// ------------------------------------------------------------------
void backpropagate(int nodeId, float leafValue)
{
    int current = nodeId;
    while (current != -1)
    {
        Node &n = g_nodes[current];
        n.visits = n.visits + 1;
        n.valueSum = n.valueSum + leafValue;
        // flip value for opponent perspective
        leafValue = -leafValue;
        // move to parent – we don't store parent; we climb by searching
        // (inefficient but acceptable for tiny demo)
        int parent = -1;
        int i = 0;
        while (i < g_nextNode)
        {
            Node &candidate = g_nodes[i];
            int j = 0;
            while (j < candidate.childCnt)
            {
                if (candidate.childIdx[j] == current)
                {
                    parent = i;
                    j = candidate.childCnt; // break inner
                }
                else
                {
                    j = j + 1;
                }
            }
            if (parent != -1) i = g_nextNode; // break outer
            else i = i + 1;
        }
        current = parent;
    }
}

// ------------------------------------------------------------------
// One MCTS iteration starting from rootId
// ------------------------------------------------------------------
void mcts_iteration(int rootId)
{
    // 1. Selection
    int node = rootId;
    while (true)
    {
        Node &n = g_nodes[node];
        // If node is terminal, stop selection
        if (board_win(n.board, 1) || board_win(n.board, -1) || board_full(n.board))
            break;
        // If node not expanded yet, expand now
        if (n.childCnt == 0)
        {
            node_expand(node);
            break;
        }
        // Otherwise select best UCT child
        int sel = node_select_uct(node);
        if (sel == -1) break;
        node = sel;
    }

    // 2. Evaluation (leaf value)
    Node &leaf = g_nodes[node];
    float leafVal = eval_board(leaf.board, leaf.player);
    // If leaf not terminal, do a rollout
    if (leafVal == 0.0f && !board_full(leaf.board))
    {
        leafVal = rollout_simulate(leaf.board, leaf.player);
    }

    // 3. Back‑propagation
    backpropagate(node, leafVal);
}

// ------------------------------------------------------------------
// Choose best move from root after many iterations
// ------------------------------------------------------------------
int best_move_from_root(int rootId)
{
    Node &root = g_nodes[rootId];
    int bestIdx = -1;
    float bestScore = -1.0e30f;

    int i = 0;
    while (i < root.childCnt)
    {
        int childId = root.childIdx[i];
        Node &c = g_nodes[childId];
        float avg = 0.0f;
        if (c.visits > 0)
        {
            avg = c.valueSum / (float)c.visits;
        }
        std::printf("Root child %d (move %d) -> visits=%d avg=%.4f\n",
                    i, c.moveFromParent, c.visits, avg);
        if (avg > bestScore)
        {
            bestScore = avg;
            bestIdx = c.moveFromParent;
        }
        i = i + 1;
    }
    return bestIdx;
}

// ------------------------------------------------------------------
// Main – deterministic test vector
// ------------------------------------------------------------------
int main()
{
    std::printf("AlphaZero simplified demo – version #10\n");

    // Minimal deterministic starting board (empty, X to move)
    int startBoard[9];
    board_clear(startBoard);

    // Print initial board
    std::printf("Initial board:\n");
    int r = 0;
    while (r < 3)
    {
        int c = 0;
        while (c < 3)
        {
            int v = startBoard[r * 3 + c];
            char ch = (v == 1) ? 'X' : (v == -1) ? 'O' : '.';
            std::printf("%c ", ch);
            c = c + 1;
        }
        std::printf("\n");
        r = r + 1;
    }

    // Create root node (player 1 = X)
    int root = node_create(startBoard, 1, -1);
    if (root == -1)
    {
        std::printf("Failed to allocate root node.\n");
        return 0;
    }

    // Run a single MCTS iteration (minimal nontrivial)
    int iter = 0;
    while (iter < 1)          // 1 iteration
    {
        std::printf("\n--- MCTS iteration %d ---\n", iter + 1);
        mcts_iteration(root);
        iter = iter + 1;
    }

    // Choose best move
    int bestMove = best_move_from_root(root);
    std::printf("\nBest move chosen (0‑based index): %d\n", bestMove);
    std::printf("Corresponding board position: row %d, col %d\n",
                bestMove / 3, bestMove % 3);

    // Apply best move to show final board
    int finalBoard[9];
    board_copy(startBoard, finalBoard);
    board_apply(finalBoard, bestMove, 1);

    std::printf("\nFinal board after best move:\n");
    r = 0;
    while (r < 3)
    {
        int c = 0;
        while (c < 3)
        {
            int v = finalBoard[r * 3 + c];
            char ch = (v == 1) ? 'X' : (v == -1) ? 'O' : '.';
            std::printf("%c ", ch);
            c = c + 1;
        }
        std::printf("\n");
        r = r + 1;
    }

    return 0;
}
