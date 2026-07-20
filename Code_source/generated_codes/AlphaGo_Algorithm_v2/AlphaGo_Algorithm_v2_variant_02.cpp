/* LLM input variant 2: small-diverse */
#include <iostream>

int main() {
    // board dimensions (small diverse)
    const int dim = 4;
    // number of simulations per move
    const int sims_per = 50;

    // ---------- deterministic board ----------
    // 0 = empty, 1 = black, -1 = white
    int board[4][4] = {
        { 0,  1, -1,  0},
        { 1,  0,  0, -1},
        {-1,  0,  1,  0},
        { 0, -1,  0,  1}
    };

    // ---------- deterministic policy priors ----------
    float policy[16] = {
        0.15f, 0.85f, 0.40f, 0.60f,
        0.70f, 0.30f, 0.55f, 0.45f,
        0.25f, 0.75f, 0.20f, 0.80f,
        0.10f, 0.90f, 0.35f, 0.65f
    };

    // ---------- MCTS simulation containers ----------
    int win_cnt[16];
    int sim_cnt[16];
    float final_score[16];
    for (int i = 0; i < dim * dim; ++i) {
        win_cnt[i] = 0;
        sim_cnt[i] = 0;
        final_score[i] = 0.0f;
    }

    // ---------- evaluate each empty cell ----------
    for (int move_idx = 0; move_idx < dim * dim; ++move_idx) {
        int mr = move_idx / dim;
        int mc = move_idx % dim;
        // only consider empty cells
        int is_empty = (board[mr][mc] == 0);
        // skip non‑empty positions without branching
        sim_cnt[move_idx] += is_empty * sims_per;

        for (int sim_loop = 0; sim_loop < sims_per; ++sim_loop) {
            // copy current board
            int sim_board[4][4];
            for (int copy_idx = 0; copy_idx < dim * dim; ++copy_idx) {
                int rr = copy_idx / dim;
                int cc = copy_idx % dim;
                sim_board[rr][cc] = board[rr][cc];
            }
            // place the candidate move
            sim_board[mr][mc] = 1; // assume black to move

            // random‑like playout until board full (deterministic order)
            for (int playout_step = 0; playout_step < dim * dim; ++playout_step) {
                int pr = playout_step / dim;
                int pc = playout_step % dim;
                int empty_now = (sim_board[pr][pc] == 0);
                int turn = ((playout_step + 1) % 2) * 2 - 1; // 1 for black, -1 for white
                sim_board[pr][pc] = empty_now * turn + sim_board[pr][pc] * (1 - empty_now);
            }

            // evaluate winner: sum > 0 => black wins
            int sum = 0;
            for (int sum_idx = 0; sum_idx < dim * dim; ++sum_idx) {
                int sr = sum_idx / dim;
                int sc = sum_idx % dim;
                sum += sim_board[sr][sc];
            }
            int black_wins = (sum > 0);
            win_cnt[move_idx] += is_empty * black_wins;
        }
        // compute win rate
        float win_rate = 0.0f;
        if (sim_cnt[move_idx] != 0) {
            win_rate = static_cast<float>(win_cnt[move_idx]) / static_cast<float>(sim_cnt[move_idx]);
        }
        // combine with policy (expanded multi‑step)
        float weighted_win = win_rate * 0.7f;
        float weighted_pol = policy[move_idx] * 0.3f;
        final_score[move_idx] = weighted_win + weighted_pol;
    }

    // ---------- select best move ----------
    int best_idx = 0;
    for (int scan = 0; scan < dim * dim; ++scan) {
        best_idx = (final_score[scan] > final_score[best_idx]) ? scan : best_idx;
    }
    int best_r = best_idx / dim;
    int best_c = best_idx % dim;
    float best_val = final_score[best_idx];

    // ---------- output ----------
    std::cout << "Best move: (" << best_r << ", " << best_c << ") ";
    std::cout << "score: " << best_val << std::endl;

    return 0;
}
