#include <iostream>
#include <array>
#include <algorithm>

#define MAX_N 36          // 6 x 6 grid
#define MAX_NEI 4         // max neighbours per node
#define INF 1000000

/* LLM input variant 2: small-diverse */

// ------------------------------------------------------------
// A* implementation (version #5) – class based, stack arrays,
// loop‑heavy, iterative, edge‑case heavy internal tests.
// ------------------------------------------------------------
class AStarPathfinder {
    // ----- static graph representation (stack memory) -----
    int edgeCnt[MAX_N];                                 // number of neighbours per node
    int neighIdx[MAX_N][MAX_NEI];                       // neighbour node indices
    int edgeCost[MAX_N][MAX_NEI];                       // cost to each neighbour (int)

    // ----- A* bookkeeping (stack arrays) -----
    int gScore[MAX_N];                                  // cost from start to node
    int fScore[MAX_N];                                  // gScore + heuristic
    int cameFrom[MAX_N];                                // predecessor node
    bool openSet[MAX_N];                                // flag: node is in open set
    bool closedSet[MAX_N];                              // flag: node already processed

    // ----- heuristic: Manhattan distance on a 6x6 grid -----
    int heuristic(int a, int b) {
        int ax = a % 6, ay = a / 6;
        int bx = b % 6, by = b / 6;
        return (ax > bx ? ax - bx : bx - ax) + (ay > by ? ay - by : by - ay);
    }

    // ----- reset all bookkeeping arrays -----
    void resetArrays() {
        for (int i = 0; i < MAX_N; ++i) {
            gScore[i] = INF;
            fScore[i] = INF;
            cameFrom[i] = -1;
            openSet[i] = false;
            closedSet[i] = false;
        }
    }

public:
    // --------------------------------------------------------
    // Build a 6x6 grid, optionally inserting obstacles.
    // obstacles[j] == true  => node j is blocked.
    // --------------------------------------------------------
    void buildGrid(const bool obstacles[MAX_N]) {
        // clear neighbour data
        for (int i = 0; i < MAX_N; ++i) edgeCnt[i] = 0;

        // for each cell, try to add up/down/left/right edges
        for (int y = 0; y < 6; ++y) {
            for (int x = 0; x < 6; ++x) {
                int cur = y * 6 + x;
                if (obstacles[cur]) continue;               // blocked node, no edges

                // four potential moves
                const int dx[4] = {1, -1, 0, 0};
                const int dy[4] = {0, 0, 1, -1};

                for (int d = 0; d < 4; ++d) {
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (nx < 0 || nx >= 6 || ny < 0 || ny >= 6) continue;
                    int nxt = ny * 6 + nx;
                    if (obstacles[nxt]) continue;           // neighbour blocked
                    // add edge cur -> nxt
                    int pos = edgeCnt[cur];
                    neighIdx[cur][pos] = nxt;
                    edgeCost[cur][pos] = 1;                  // uniform cost
                    ++edgeCnt[cur];
                }
            }
        }
    }

    // --------------------------------------------------------
    // Perform A* search from startIdx to goalIdx.
    // Returns true if a path is found, false otherwise.
    // --------------------------------------------------------
    bool search(int startIdx, int goalIdx, int outPath[MAX_N], int &outLen, int &outCost) {
        resetArrays();

        // initialise start node
        gScore[startIdx] = 0;
        fScore[startIdx] = heuristic(startIdx, goalIdx);
        openSet[startIdx] = true;

        // ----- main loop (no recursion) -----
        while (true) {
            // find node in open set with lowest fScore
            int bestNode = -1;
            int bestF = INF;
            for (int i = 0; i < MAX_N; ++i) {
                if (openSet[i] && fScore[i] < bestF) {
                    bestF = fScore[i];
                    bestNode = i;
                }
            }

            // no node left => failure
            if (bestNode == -1) break;

            // reached goal?
            if (bestNode == goalIdx) {
                // reconstruct path
                outLen = 0;
                int trace = goalIdx;
                while (trace != -1) {
                    outPath[outLen++] = trace;
                    trace = cameFrom[trace];
                }
                // reverse path order
                for (int i = 0; i < outLen / 2; ++i)
                    std::swap(outPath[i], outPath[outLen - 1 - i]);

                outCost = gScore[goalIdx];
                return true;
            }

            // move bestNode from open to closed
            openSet[bestNode] = false;
            closedSet[bestNode] = true;

            // examine neighbours
            int neighbours = edgeCnt[bestNode];
            for (int n = 0; n < neighbours; ++n) {
                int nxt = neighIdx[bestNode][n];
                if (closedSet[nxt]) continue;                // already processed

                int tentativeG = gScore[bestNode] + edgeCost[bestNode][n];
                // if nxt not in open set, add it
                if (!openSet[nxt]) {
                    openSet[nxt] = true;
                }
                // ignore if not a better path
                if (tentativeG >= gScore[nxt]) continue;

                // this path is the best so far
                cameFrom[nxt] = bestNode;
                gScore[nxt] = tentativeG;
                // recompute fScore after updating gScore
                fScore[nxt] = gScore[nxt] + heuristic(nxt, goalIdx);
            }
        }

        // no path found
        outLen = 0;
        outCost = INF;
        return false;
    }
};

// ------------------------------------------------------------
// Helper: print a path stored in an array.
// ------------------------------------------------------------
void showPath(const int path[], int length, int cost) {
    std::cout << "Path length: " << length << "\n";
    std::cout << "Total cost : " << cost << "\n";
    std::cout << "Sequence   : ";
    for (int i = 0; i < length; ++i) {
        int node = path[i];
        std::cout << "(" << node % 6 << "," << node / 6 << ")";
        if (i + 1 < length) std::cout << " -> ";
    }
    std::cout << "\n";
}

// ------------------------------------------------------------
// Main – runs several edge‑case heavy scenarios.
// ------------------------------------------------------------
int main() {
    AStarPathfinder solver;
    bool blockMap[MAX_N] = { false };

    // ---------- Scenario 1: open grid, offset start/goal ----------
    std::fill(blockMap, blockMap + MAX_N, false);
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        int start = 1 * 6 + 1;   // (1,1)
        int goal  = 4 * 6 + 4;   // (4,4)
        std::cout << "Scenario 1: Open grid, start (1,1) -> goal (4,4)\n";
        bool ok = solver.search(start, goal, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 2: start equals goal ----------
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        int same = 5 * 6 + 0;   // (0,5)
        std::cout << "Scenario 2: Start equals Goal (0,5)\n";
        bool ok = solver.search(same, same, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 3: blocked start ----------
    std::fill(blockMap, blockMap + MAX_N, false);
    blockMap[7] = true;                     // block (1,1) which will be start
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        int start = 7;                      // (1,1) blocked
        int goal = 35;                      // (5,5)
        std::cout << "Scenario 3: Start blocked at (1,1)\n";
        bool ok = solver.search(start, goal, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 4: zig‑zag corridor ----------
    std::fill(blockMap, blockMap + MAX_N, false);
    // block columns 1 and 4 except a winding path
    for (int y = 0; y < 6; ++y) {
        if (y != 0 && y != 5) blockMap[y * 6 + 1] = true; // column 1 blocked except top/bottom
        if (y != 2 && y != 3) blockMap[y * 6 + 4] = true; // column 4 blocked except rows 2,3
    }
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 4: Zig‑zag corridor, start (0,0) -> goal (5,5)\n";
        bool ok = solver.search(0, 35, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 5: scattered obstacles, multiple routes ----------
    std::fill(blockMap, blockMap + MAX_N, false);
    int scattered[] = {5, 12, 13, 14, 19, 20, 27, 33}; // arbitrary blocked cells
    for (int idx : scattered) blockMap[idx] = true;
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 5: Scattered obstacles, start (0,0) -> goal (5,5)\n";
        bool ok = solver.search(0, 35, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    return 0;
}
