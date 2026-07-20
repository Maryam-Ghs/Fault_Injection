/* LLM input variant 2: small-diverse */
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

// ------------------------------------------------------------
// AlphaZero simplified implementation (Version #8)
// ------------------------------------------------------------
// This example uses a tiny Tic‑Tac‑Toe board (3x3) and a
// minimal neural network (2‑layer) with only float/int types.
// All data are stored on the stack; no dynamic allocation.
// ------------------------------------------------------------

class AlphaZeroMini {
public:
    // Game constants
    int board_dim;          // board side length (3)
    int cells_total;        // total cells (9)

    // Neural network parameters (tiny two‑layer net)
    static const int hidden_sz_const = 8; // hidden layer size (8)
    int hidden_sz;          // runtime copy of hidden size

    float net_w1[9 * hidden_sz_const];   // weights input → hidden
    float net_b1[hidden_sz_const];       // bias hidden
    float net_w2[hidden_sz_const * 10];  // weights hidden → (policy+value)
    float net_b2[10];                    // bias output (9 policy + 1 value)

    // Constructor: initialise network with deterministic small numbers
    AlphaZeroMini() {
        board_dim = 3;
        cells_total = board_dim * board_dim;
        hidden_sz = hidden_sz_const;

        // Seed RNG with fixed seed for deterministic behaviour
        std::srand(42);

        // Initialise first layer
        for (int i = 0; i < cells_total * hidden_sz; ++i) {
            net_w1[i] = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.1f;
        }
        for (int i = 0; i < hidden_sz; ++i) {
            net_b1[i] = 0.0f;
        }

        // Initialise second layer
        for (int i = 0; i < hidden_sz * (cells_total + 1); ++i) {
            net_w2[i] = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.1f;
        }
        for (int i = 0; i < cells_total + 1; ++i) {
            net_b2[i] = 0.0f;
        }
    }

    // --------------------------------------------------------
    // 1. Forward pass of the tiny network
    //    Input : board_state (int[9])  -> -1,0,1
    //    Output: policy[9] (float)   -> probabilities
    //            value  (float)       -> board evaluation
    // --------------------------------------------------------
    void evaluate(const int board_state[9], float policy[9], float& value) {
        // Normalise board to float in [-1,1]
        float inp[9];
        for (int i = 0; i < cells_total; ++i) {
            inp[i] = static_cast<float>(board_state[i]);
        }

        // Hidden layer linear combination + ReLU
        float hidden[hidden_sz_const];
        for (int h = 0; h < hidden_sz; ++h) {
            float acc = net_b1[h];
            for (int j = 0; j < cells_total; ++j) {
                acc += net_w1[j * hidden_sz + h] * inp[j];
            }
            // ReLU
            hidden[h] = (acc > 0.0f) ? acc : 0.0f;
        }

        // Output layer linear combination
        float out_raw[10];
        for (int o = 0; o < cells_total + 1; ++o) {
            float acc = net_b2[o];
            for (int h = 0; h < hidden_sz; ++h) {
                acc += net_w2[h * (cells_total + 1) + o] * hidden[h];
            }
            out_raw[o] = acc;
        }

        // Softmax for policy (only on legal moves later)
        float max_logit = out_raw[0];
        for (int i = 1; i < cells_total; ++i) {
            if (out_raw[i] > max_logit) max_logit = out_raw[i];
        }
        float sum_exp = 0.0f;
        for (int i = 0; i < cells_total; ++i) {
            float e = std::exp(out_raw[i] - max_logit);
            policy[i] = e;
            sum_exp += e;
        }
        for (int i = 0; i < cells_total; ++i) {
            policy[i] /= sum_exp;
        }

        // Value is a tanh activation to keep it in [-1,1]
        float v_raw = out_raw[cells_total];
        // Simple tanh approximation
        if (v_raw > 3.0f) v_raw = 3.0f;
        if (v_raw < -3.0f) v_raw = -3.0f;
        value = std::tanh(v_raw);
    }

    // --------------------------------------------------------
    // 2. MCTS node statistics stored on the stack
    // --------------------------------------------------------
    struct NodeStat {
        int    visit_cnt;
        float  total_val;
        float  prior_prob;
    };

    // --------------------------------------------------------
    // 3. Perform MCTS simulations for the current player
    // --------------------------------------------------------
    void runMCTS(const int board_state[9], int player, int simulations,
                 float final_policy[9]) {
        // Initialise statistics for each move
        NodeStat stats[9];
        for (int i = 0; i < cells_total; ++i) {
            stats[i].visit_cnt  = 0;
            stats[i].total_val  = 0.0f;
            stats[i].prior_prob = 0.0f;
        }

        // Get network prior + value for root
        float root_policy[9];
        float root_value;
        evaluate(board_state, root_policy, root_value);

        // Fill priors only for legal actions
        for (int i = 0; i < cells_total; ++i) {
            if (board_state[i] == 0) {
                stats[i].prior_prob = root_policy[i];
            }
        }

        // ----- Simulation loop (changed to while) -----
        int sim = 0;
        while (sim < simulations) {
            // ----- Selection -----
            int chosen = -1;
            float best_uct = -1.0f;
            for (int i = 0; i < cells_total; ++i) {
                if (board_state[i] != 0) continue; // illegal
                // UCT = Q + c * P * sqrt(N)/ (1 + n)
                float q = (stats[i].visit_cnt == 0) ? 0.0f :
                          stats[i].total_val / static_cast<float>(stats[i].visit_cnt);
                float u = 1.414f * stats[i].prior_prob *
                          std::sqrt(static_cast<float>(sim + 1)) /
                          (1.0f + static_cast<float>(stats[i].visit_cnt));
                float score = q + u;
                if (score > best_uct) {
                    best_uct = score;
                    chosen = i;
                }
            }

            // ----- Expansion & Evaluation -----
            int next_state[9];
            for (int i = 0; i < cells_total; ++i) {
                next_state[i] = board_state[i];
            }
            next_state[chosen] = player;

            float leaf_policy[9];
            float leaf_value;
            evaluate(next_state, leaf_policy, leaf_value);

            // ----- Backpropagation -----
            // For Tic‑Tac‑Toe we treat leaf_value from the perspective of the player who just moved
            float value_to_back = leaf_value * static_cast<float>(player);
            for (int i = 0; i < cells_total; ++i) {
                if (board_state[i] != 0) continue; // only visited node
                if (i == chosen) {
                    stats[i].visit_cnt += 1;
                    stats[i].total_val += value_to_back;
                }
            }

            ++sim;
        }

        // ----- Derive final policy from visit counts (reordered) -----
        int total_visits = 0;
        for (int i = 0; i < cells_total; ++i) total_visits += stats[i].visit_cnt;
        for (int i = 0; i < cells_total; ++i) {
            if (total_visits == 0) {
                final_policy[i] = 0.0f;
            } else {
                final_policy[i] = static_cast<float>(stats[i].visit_cnt) / static_cast<float>(total_visits);
            }
        }
    }

    // --------------------------------------------------------
    // 4. Self‑play a single game
    // --------------------------------------------------------
    int playGame() {
        int board[9];
        for (int i = 0; i < cells_total; ++i) board[i] = 0;

        int current_player = 1; // player 1 starts (uses 1, opponent -1)
        int move_number = 0;
        while (true) {
            // MCTS to obtain move probabilities
            float move_prob[9];
            runMCTS(board, current_player, 12, move_prob); // reduced simulations

            // Choose move with highest probability (deterministic for demo)
            int best_move = -1;
            float best_p = -1.0f;
            for (int i = 0; i < cells_total; ++i) {
                if (board[i] != 0) continue;
                if (move_prob[i] > best_p) {
                    best_p = move_prob[i];
                    best_move = i;
                }
            }

            // Apply move
            board[best_move] = current_player;
            ++move_number;

            // Print board after each move
            std::cout << "Move " << move_number << " by player " << current_player << ":\n";
            for (int r = 0; r < board_dim; ++r) {
                for (int c = 0; c < board_dim; ++c) {
                    int v = board[r * board_dim + c];
                    char ch = (v == 1) ? 'X' : (v == -1) ? 'O' : '.';
                    std::cout << ch << ' ';
                }
                std::cout << '\n';
            }
            std::cout << "-------------------\n";

            // Check for win or draw
            int result = checkWinner(board);
            if (result != 0) {
                std::cout << "Player " << ((result == 1) ? "X" : "O") << " wins!\n";
                return result;
            }
            if (move_number == cells_total) {
                std::cout << "Game ends in a draw.\n";
                return 0;
            }

            // Switch player
            current_player = -current_player;
        }
    }

    // --------------------------------------------------------
    // 5. Simple winner detection for Tic‑Tac‑Toe
    // --------------------------------------------------------
    int checkWinner(const int board[9]) {
        // Rows
        for (int r = 0; r < board_dim; ++r) {
            int sum = board[r * board_dim] + board[r * board_dim + 1] + board[r * board_dim + 2];
            if (sum == 3) return 1;
            if (sum == -3) return -1;
        }
        // Columns
        for (int c = 0; c < board_dim; ++c) {
            int sum = board[c] + board[c + 3] + board[c + 6];
            if (sum == 3) return 1;
            if (sum == -3) return -1;
        }
        // Diagonals
        int d1 = board[0] + board[4] + board[8];
        int d2 = board[2] + board[4] + board[6];
        if (d1 == 3 || d2 == 3) return 1;
        if (d1 == -3 || d2 == -3) return -1;

        return 0; // no winner yet
    }
};

// ------------------------------------------------------------
// Entry point
// ------------------------------------------------------------
int main() {
    AlphaZeroMini engine;
    engine.playGame();
    return 0;
}
