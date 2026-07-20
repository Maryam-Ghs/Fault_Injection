#include <iostream>
#include <cstdlib>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // -------------------  Input (duplicate‑heavy predefined grid) -------------------
    int w = 5;                     // width of the grid
    int h = 5;                     // height of the grid

    // 0 = free, 1 = blocked (repeated pattern)
    int terrain[5][5] = {
        {0, 1, 0, 1, 0},
        {0, 1, 0, 1, 0},
        {0, 1, 0, 1, 0},
        {0, 1, 0, 1, 0},
        {0, 1, 0, 1, 0}
    };

    int srcX = 0, srcY = 0;        // start cell
    int dstX = 4, dstY = 4;        // destination cell

    // -------------------  Memory (stack arrays) -------------------
    int gCost[5][5];               // cost from start
    float fCost[5][5];             // estimated total cost (g + h)
    int parentX[5][5];             // back‑track X coordinate
    int parentY[5][5];             // back‑track Y coordinate
    bool closedSet[5][5];          // visited cells
    bool openSet[5][5];            // cells to be examined

    // -------------------  Initialization -------------------
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            gCost[i][j] = 10000;          // a large number as "infinity"
            fCost[i][j] = 10000.0f;
            parentX[i][j] = -1;
            parentY[i][j] = -1;
            closedSet[i][j] = false;
            openSet[i][j] = false;
        }
    }

    gCost[srcY][srcX] = 0;
    // heuristic = Manhattan distance
    int hEst = (dstX > srcX ? dstX - srcX : srcX - dstX) +
               (dstY > srcY ? dstY - srcY : srcY - dstY);
    fCost[srcY][srcX] = static_cast<float>(hEst);
    openSet[srcY][srcX] = true;

    // -------------------  A* Main Loop (iterative, loop‑heavy) -------------------
    while (true) {
        // ---- find node with minimum fCost in openSet ----
        int curX = -1, curY = -1;
        float bestF = 100000.0f;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (openSet[y][x]) {
                    if (fCost[y][x] < bestF) {
                        bestF = fCost[y][x];
                        curX = x;
                        curY = y;
                    }
                }
            }
        }

        // ---- if no node found, break (no path) ----
        if (curX == -1) {
            std::cout << "No path found.\n";
            break;
        }

        // ---- goal reached? ----
        if (curX == dstX && curY == dstY) {
            // reconstruct path
            int pathLen = 0;
            int pathX[25];
            int pathY[25];
            int tx = dstX, ty = dstY;
            while (tx != -1 && ty != -1) {
                pathX[pathLen] = tx;
                pathY[pathLen] = ty;
                ++pathLen;
                int nx = parentX[ty][tx];
                int ny = parentY[ty][tx];
                tx = nx;
                ty = ny;
            }
            std::cout << "Path length: " << pathLen - 1 << "\n";
            std::cout << "Path (from start to goal):\n";
            for (int i = pathLen - 1; i >= 0; --i) {
                std::cout << "(" << pathX[i] << "," << pathY[i] << ")";
                if (i) std::cout << " -> ";
            }
            std::cout << "\n";
            break;
        }

        // ---- move current node from openSet to closedSet ----
        openSet[curY][curX] = false;
        closedSet[curY][curX] = true;

        // ---- examine 4‑neighbourhood ----
        int delta[4][2] = {{0,1},{1,0},{0,-1},{-1,0}};
        for (int d = 0; d < 4; ++d) {
            int nxtX = curX + delta[d][0];
            int nxtY = curY + delta[d][1];

            // boundary check
            if (nxtX < 0 || nxtX >= w || nxtY < 0 || nxtY >= h)
                continue;
            // obstacle check
            if (terrain[nxtY][nxtX] == 1)
                continue;
            // already processed
            if (closedSet[nxtY][nxtX])
                continue;

            // ---- tentative gCost (multi‑step) ----
            int baseG = gCost[curY][curX];
            int stepG = baseG + 1;               // cost to move is 1
            int candG = stepG;

            // ---- heuristic (Manhattan) ----
            int diffX = nxtX > dstX ? nxtX - dstX : dstX - nxtX;
            int diffY = nxtY > dstY ? nxtY - dstY : dstY - nxtY;
            int heur = diffX + diffY;

            // ---- fCost = g + h (expanded) ----
            int sumGH = candG + heur;
            float candF = static_cast<float>(sumGH);

            // ---- if better path found, update structures ----
            if (candG < gCost[nxtY][nxtX]) {
                parentX[nxtY][nxtX] = curX;
                parentY[nxtY][nxtX] = curY;
                gCost[nxtY][nxtX] = candG;
                fCost[nxtY][nxtX] = candF;
                openSet[nxtY][nxtX] = true;
            }
        }
    }

    return 0;
}
