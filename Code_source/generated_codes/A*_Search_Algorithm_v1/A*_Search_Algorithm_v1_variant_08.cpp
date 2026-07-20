#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

int main() {
    // ---------- grid definition ----------
    // 0 = free cell, 1 = obstacle
    const int rows = 20, cols = 20;               // larger sparse grid
    std::vector<std::vector<int>> board(rows, std::vector<int>(cols, 0));

    // clustered obstacles (sparse overall)
    for (int r = 5; r <= 8; ++r) {
        for (int c = 5; c <= 8; ++c) {
            board[r][c] = 1;
        }
    }
    // a few additional obstacles far from start/goal to keep skewed sparsity
    board[12][3] = 1;
    board[14][15] = 1;
    board[3][14] = 1;
    board[17][7] = 1;

    // ---------- start / goal ----------
    int startR = 0, startC = 0;
    int goalR  = rows - 1, goalC  = cols - 1;

    // ---------- node description (local class) ----------
    struct Cell {
        int r, c;          // position
        int pr, pc;        // parent position (-1,-1) for start
        float g, h, f;     // costs
    };

    // ---------- containers ----------
    std::vector<Cell> openSet;
    std::vector<Cell> closedSet;

    // ---------- heuristic (Manhattan) ----------
    auto heuristic = [&](int r, int c) -> float {
        return std::abs(r - goalR) + std::abs(c - goalC);
    };

    // ---------- seed the search ----------
    Cell first;
    first.r = startR; first.c = startC;
    first.pr = -1;    first.pc = -1;
    first.g = 0.0f;
    first.h = heuristic(startR, startC);
    first.f = first.g + first.h;
    openSet.push_back(first);

    // ---------- A* main loop ----------
    while (!openSet.empty()) {
        // pick node with smallest f
        int bestIdx = 0;
        for (int i = 1; i < (int)openSet.size(); ++i)
            if (openSet[i].f < openSet[bestIdx].f) bestIdx = i;
        Cell cur = openSet[bestIdx];
        openSet.erase(openSet.begin() + bestIdx);
        closedSet.push_back(cur);

        // reached goal ?
        if (cur.r == goalR && cur.c == goalC) {
            // reconstruct path
            std::vector<std::pair<int,int>> path;
            int tr = cur.r, tc = cur.c;
            int pr = cur.pr, pc = cur.pc;
            path.emplace_back(tr, tc);
            while (pr != -1 && pc != -1) {
                path.emplace_back(pr, pc);
                // find parent in closedSet
                for (auto &node : closedSet)
                    if (node.r == pr && node.c == pc) {
                        tr = pr; tc = pc;
                        pr = node.pr; pc = node.pc;
                        break;
                    }
            }
            std::reverse(path.begin(), path.end());
            std::cout << "Path length: " << (int)path.size() - 1 << "\n";
            std::cout << "Path:\n";
            for (auto &p : path)
                std::cout << "(" << p.first << "," << p.second << ") ";
            std::cout << "\n";
            return 0;
        }

        // explore 4-neighbourhood
        const int dr[4] = {-1, 1, 0, 0};
        const int dc[4] = {0, 0, -1, 1};
        for (int d = 0; d < 4; ++d) {
            int nr = cur.r + dr[d];
            int nc = cur.c + dc[d];

            // bounds & obstacle test
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
            if (board[nr][nc] == 1) continue;

            // already closed ?
            bool inClosed = false;
            for (auto &node : closedSet)
                if (node.r == nr && node.c == nc) { inClosed = true; break; }
            if (inClosed) continue;

            // compute tentative g, h, f
            float tentativeG = cur.g + 1.0f;                 // movement cost = 1
            float hVal = heuristic(nr, nc);
            float fVal = tentativeG + hVal;

            // check if node already in open set with a better g
            bool better = true;
            for (auto &node : openSet)
                if (node.r == nr && node.c == nc) {
                    if (tentativeG >= node.g) better = false;
                    break;
                }
            if (!better) continue;

            // add / update node in open set
            Cell nxt;
            nxt.r = nr; nxt.c = nc;
            nxt.pr = cur.r; nxt.pc = cur.c;
            nxt.g = tentativeG;
            nxt.h = hVal;
            nxt.f = fVal;
            openSet.push_back(nxt);
        }
    }

    std::cout << "No path found.\n";
    return 0;
}
