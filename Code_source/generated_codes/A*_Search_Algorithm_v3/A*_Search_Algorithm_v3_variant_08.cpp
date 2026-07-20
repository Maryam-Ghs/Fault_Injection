/* LLM input variant 8: sparse-skewed */
// A* Search – version 3
// ------------------------------------------------------------
// Implements A* on a fixed‑size grid using only int/float,
// stack‑allocated arrays, and a class‑based design.
// ------------------------------------------------------------

#include <iostream>
#include <cmath>   // only for std::abs (works with int)
#include <algorithm>

#define GRID_W 15
#define GRID_H 15
#define MAX_NODES (GRID_W * GRID_H)

// -----------------------------------------------------------------
// Simple node representation stored in a flat array
// -----------------------------------------------------------------
struct Cell {
    int ix;          // x coordinate
    int iy;          // y coordinate
    float gcost;     // cost from start
    float hcost;     // heuristic to goal
    float fcost;     // g + h
    int parent;      // index of parent node in node pool, -1 if none
    bool opened;     // is in open set?
    bool closed;     // is in closed set?
};

// -----------------------------------------------------------------
// A* algorithm encapsulated in a class
// -----------------------------------------------------------------
class Pathfinder {
    Cell pool[MAX_NODES];          // static node pool
    int openIdx[MAX_NODES];        // indices of nodes in open set
    int openCnt;                   // number of open nodes
    int startX, startY;
    int goalX, goalY;
    int obstacle[GRID_H][GRID_W]; // 0 = free, 1 = blocked

public:
    Pathfinder() : openCnt(0), startX(0), startY(0), goalX(0), goalY(0) {
        // initialise node pool
        for (int i = 0; i < MAX_NODES; ++i) {
            pool[i].ix = i % GRID_W;
            pool[i].iy = i / GRID_W;
            pool[i].gcost = 0.0f;
            pool[i].hcost = 0.0f;
            pool[i].fcost = 0.0f;
            pool[i].parent = -1;
            pool[i].opened = false;
            pool[i].closed = false;
        }
        // sparse obstacle layout: a few isolated blocks
        for (int y = 0; y < GRID_H; ++y) {
            for (int x = 0; x < GRID_W; ++x) {
                obstacle[y][x] = 0;
            }
        }
        // place a few scattered obstacles
        obstacle[5][5] = 1;
        obstacle[10][3] = 1;
        obstacle[12][12] = 1;
        // start and goal far apart
        startX = 0; startY = 0;
        goalX  = GRID_W - 1;
        goalY  = GRID_H - 1;
    }

    // -----------------------------------------------------------------
    // Manhattan distance heuristic (expanded into multi‑step form)
    // -----------------------------------------------------------------
    float estimate(int ax, int ay) {
        int dx = ax - goalX;
        int dy = ay - goalY;
        int absdx = dx < 0 ? -dx : dx;
        int absdy = dy < 0 ? -dy : dy;
        float step1 = (float)absdx;
        float step2 = (float)absdy;
        float h = step1 + step2;
        return h;
    }

    // -----------------------------------------------------------------
    // Convert (x,y) to node index in the pool
    // -----------------------------------------------------------------
    int indexOf(int ax, int ay) {
        return ay * GRID_W + ax;
    }

    // -----------------------------------------------------------------
    // Add a neighbour to the open set if it improves the path
    // -----------------------------------------------------------------
    void tryNeighbour(int curIdx, int nx, int ny) {
        if (nx < 0 || ny < 0 || nx >= GRID_W || ny >= GRID_H) return;
        if (obstacle[ny][nx] == 1) return; // blocked

        int nIdx = indexOf(nx, ny);
        Cell &cur = pool[curIdx];
        Cell &nbr = pool[nIdx];

        if (nbr.closed) return; // already processed

        // step‑by‑step cost computation
        float stepCost = 1.0f;          // uniform grid cost
        float tentativeG = cur.gcost + stepCost;

        bool better = false;
        if (!nbr.opened) {
            // first time we see this node
            nbr.hcost = estimate(nx, ny);
            nbr.opened = true;
            better = true;
        } else if (tentativeG < nbr.gcost) {
            // found a cheaper path to an already opened node
            better = true;
        }

        if (better) {
            nbr.parent = curIdx;
            nbr.gcost = tentativeG;
            // recompute f after updating g and h
            float sumGH = nbr.gcost + nbr.hcost;
            nbr.fcost = sumGH;
            // add to open list if not already there
            if (nbr.opened && !nbr.closed) {
                bool exists = false;
                for (int k = 0; k < openCnt; ++k) {
                    if (openIdx[k] == nIdx) { exists = true; break; }
                }
                if (!exists) {
                    openIdx[openCnt++] = nIdx;
                }
            }
        }
    }

    // -----------------------------------------------------------------
    // Main A* loop – uses a linear scan to pick the node with lowest f
    // -----------------------------------------------------------------
    bool search() {
        int startIdx = indexOf(startX, startY);
        Cell &startNode = pool[startIdx];
        startNode.gcost = 0.0f;
        startNode.hcost = estimate(startX, startY);
        startNode.fcost = startNode.gcost + startNode.hcost;
        startNode.opened = true;
        openIdx[openCnt++] = startIdx;

        while (openCnt > 0) {
            // ---------------------------------------------------------
            // pick node with smallest fcost (linear search)
            // ---------------------------------------------------------
            int bestPos = 0;
            float bestF = pool[openIdx[0]].fcost;
            for (int i = 1; i < openCnt; ++i) {
                float curF = pool[openIdx[i]].fcost;
                if (curF < bestF) {
                    bestF = curF;
                    bestPos = i;
                }
            }
            int curIdx = openIdx[bestPos];
            // remove from open list by swapping with last
            openIdx[bestPos] = openIdx[--openCnt];

            Cell &current = pool[curIdx];
            current.closed = true;

            // goal reached?
            if (current.ix == goalX && current.iy == goalY) {
                return true;
            }

            // explore 4‑directional neighbours
            tryNeighbour(curIdx, current.ix + 1, current.iy);
            tryNeighbour(curIdx, current.ix - 1, current.iy);
            tryNeighbour(curIdx, current.ix, current.iy + 1);
            tryNeighbour(curIdx, current.ix, current.iy - 1);
        }
        return false; // open set exhausted, no path
    }

    // -----------------------------------------------------------------
    // Reconstruct path from goal back to start
    // -----------------------------------------------------------------
    void outputPath() {
        int goalIdx = indexOf(goalX, goalY);
        if (!pool[goalIdx].opened) {
            std::cout << "No path found.\n";
            return;
        }
        // collect coordinates in reverse order
        int pathLen = 0;
        int revX[MAX_NODES];
        int revY[MAX_NODES];
        int cur = goalIdx;
        while (cur != -1) {
            revX[pathLen] = pool[cur].ix;
            revY[pathLen] = pool[cur].iy;
            ++pathLen;
            cur = pool[cur].parent;
        }
        // print forward
        std::cout << "Path length: " << pathLen - 1 << "\n";
        std::cout << "Path (x y): ";
        for (int i = pathLen - 1; i >= 0; --i) {
            std::cout << revX[i] << " " << revY[i];
            if (i > 0) std::cout << " -> ";
        }
        std::cout << "\n";
    }
};

// -----------------------------------------------------------------
// Entry point – generate internal grid, run A* and print result
// -----------------------------------------------------------------
int main() {
    Pathfinder explorer;
    bool found = explorer.search();
    if (found) {
        std::cout << "Path found!\n";
    } else {
        std::cout << "Failed to reach goal.\n";
    }
    explorer.outputPath();
    return 0;
}
