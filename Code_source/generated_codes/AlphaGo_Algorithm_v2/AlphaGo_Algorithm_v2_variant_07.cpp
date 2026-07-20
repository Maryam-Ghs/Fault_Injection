/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    // board dimensions (medium size)
    int dim = 5;
    // number of simulations per move
    int sims_per = 120;
    // random seed
    std::srand((unsigned)std::time(0));

    // ---------- generate deterministic board (all empty) ----------
    // 0 = empty, 1 = black, -1 = white
    int board[5][5];
    int cell_idx = 0;
    while (cell_idx < dim * dim) {
        int r = cell_idx / dim;
        int c = cell_idx % dim;
        board[r][c] = 0; // force empty cells for worst‑case full move set
        cell_idx = cell_idx + 1;
    }

    // ---------- generate reverse‑ordered policy priors ----------
    float policy[25];
    int p_idx = 0;
    while (p_idx < dim * dim) {
        // descending values from (dim*dim-1)/dim*dim to 0
        policy[p_idx] = (float)(dim * dim - 1 - p_idx) / (float)(dim * dim);
        p_idx = p_idx + 1;
    }

    // ---------- MCTS simulation containers ----------
    int win_cnt[25];
    int sim_cnt[25];
    float final_score[25];
    int i = 0;
    while (i < dim * dim) {
        win_cnt[i] = 0;
        sim_cnt[i] = 0;
        final_score[i] = 0.0f;
        i = i + 1;
    }

    // ---------- evaluate each empty cell ----------
    int move_idx = 0;
    while (move_idx < dim * dim) {
        int mr = move_idx / dim;
        int mc = move_idx % dim;
        // only consider empty cells
        int is_empty = (board[mr][mc] == 0);
        // skip non‑empty positions without branching
        sim_cnt[move_idx] = sim_cnt[move_idx] + is_empty * sims_per;
        int sim_loop = 0;
        while (sim_loop < sims_per) {
            // copy current board
            int sim_board[5][5];
            int copy_idx = 0;
            while (copy_idx < dim * dim) {
                int rr = copy_idx / dim;
                int cc = copy_idx % dim;
                sim_board[rr][cc] = board[rr][cc];
                copy_idx = copy_idx + 1;
            }
            // place the candidate move
            sim_board[mr][mc] = 1; // assume black to move

            // random playout until board full
            int playout_step = 0;
            while (playout_step < dim * dim) {
                int pr = playout_step / dim;
                int pc = playout_step % dim;
                int empty_now = (sim_board[pr][pc] == 0);
                int turn = ((playout_step + 1) % 2) * 2 - 1; // 1 for black, -1 for white
                sim_board[pr][pc] = empty_now * turn + sim_board[pr][pc] * (1 - empty_now);
                playout_step = playout_step + 1;
            }

            // evaluate winner: sum > 0 => black wins
            int sum = 0;
            int sum_idx = 0;
            while (sum_idx < dim * dim) {
                int sr = sum_idx / dim;
                int sc = sum_idx % dim;
                sum = sum + sim_board[sr][sc];
                sum_idx = sum_idx + 1;
            }
            int black_wins = (sum > 0);
            win_cnt[move_idx] = win_cnt[move_idx] + is_empty * black_wins;
            sim_loop = sim_loop + 1;
        }
        // compute win rate
        float win_rate = 0.0f;
        win_rate = (float)win_cnt[move_idx] / (float)sim_cnt[move_idx];
        // combine with policy (expanded multi‑step)
        float weighted_win = win_rate * 0.7f;
        float weighted_pol = policy[move_idx] * 0.3f;
        final_score[move_idx] = weighted_win + weighted_pol;
        move_idx = move_idx + 1;
    }

    // ---------- select best move ----------
    int best_idx = 0;
    int scan = 0;
    while (scan < dim * dim) {
        // use ternary to avoid explicit branch
        best_idx = (final_score[scan] > final_score[best_idx]) ? scan : best_idx;
        scan = scan + 1;
    }
    int best_r = best_idx / dim;
    int best_c = best_idx % dim;
    float best_val = final_score[best_idx];

    // ---------- output ----------
    std::cout << "Best move: (" << best_r << ", " << best_c << ") ";
    std::cout << "score: " << best_val << std::endl;

    return 0;
}
