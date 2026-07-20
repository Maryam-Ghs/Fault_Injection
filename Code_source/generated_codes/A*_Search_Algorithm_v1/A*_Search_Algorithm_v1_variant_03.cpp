#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

/* LLM input variant 3: zeros-and-ones */

int main() {
    // ---------- grid definition ----------
    // 0 = free cell, 1 = obstacle
    std::vector<std::vector<int>> board = {
        {0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0}
    };
    const int rows = 5, cols = 5;               // small predefined size

    // ---------- start / goal ----------
    int startR = 2, startC = 2;
    int goalR  = 2, goalC  = 2;

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
