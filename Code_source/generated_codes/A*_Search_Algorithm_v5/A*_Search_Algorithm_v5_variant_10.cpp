#include <iostream>
#include <array>
#include <algorithm>

#define GRID_W 20
#define GRID_H 20
#define MAX_N (GRID_W * GRID_H)   // 20 x 20 grid
#define MAX_NEI 4                 // max neighbours per node
#define INF 1000000

/* LLM input variant 10: large-safe-stress */

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

    // ----- heuristic: Manhattan distance on a GRID_W x GRID_H grid -----
    int heuristic(int a, int b) {
        int ax = a % GRID_W, ay = a / GRID_W;
        int bx = b % GRID_W, by = b / GRID_W;
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
    // Build a GRID_W x GRID_H grid, optionally inserting obstacles.
    // obstacles[j] == true  => node j is blocked.
    // --------------------------------------------------------
    void buildGrid(const bool obstacles[MAX_N]) {
        // clear neighbour data
        for (int i = 0; i < MAX_N; ++i) edgeCnt[i] = 0;

        // for each cell, try to add up/down/left/right edges
        for (int y = 0; y < GRID_H; ++y) {
            for (int x = 0; x < GRID_W; ++x) {
                int cur = y * GRID_W + x;
                if (obstacles[cur]) continue;               // blocked node, no edges

                // four potential moves
                const int dx[4] = {1, -1, 0, 0};
                const int dy[4] = {0, 0, 1, -1};

                for (int d = 0; d < 4; ++d) {
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) continue;
                    int nxt = ny * GRID_W + nx;
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
        std::cout << "(" << node % GRID_W << "," << node / GRID_W << ")";
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

    // ---------- Scenario 1: simple open grid ----------
    std::fill(blockMap, blockMap + MAX_N, false);
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 1: Open " << GRID_W << "x" << GRID_H << " grid, start (0,0) -> goal (" << GRID_W-1 << "," << GRID_H-1 << ")\n";
        bool ok = solver.search(0, MAX_N - 1, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 2: start equals goal ----------
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        int sx = 5, sy = 7;
        int same = sy * GRID_W + sx;               // node index for (5,7)
        std::cout << "Scenario 2: Start equals Goal (" << sx << "," << sy << ")\n";
        bool ok = solver.search(same, same, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 3: blocked goal ----------
    std::fill(blockMap, blockMap + MAX_N, false);
    blockMap[MAX_N - 1] = true;                    // block cell (GRID_W-1, GRID_H-1)
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 3: Goal blocked at (" << GRID_W-1 << "," << GRID_H-1 << ")\n";
        bool ok = solver.search(0, MAX_N - 1, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 4: narrow corridor ----------
    std::fill(blockMap, blockMap + MAX_N, false);
    // create a wall leaving a single corridor
    for (int y = 0; y < GRID_H; ++y) {
        if (y != 10) blockMap[y * GRID_W + 10] = true;   // block column 10 except row 10
        if (y != 9)  blockMap[y * GRID_W + 11] = true;   // block column 11 except row 9
    }
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 4: Narrow corridor, start (0,0) -> goal (" << GRID_W-1 << "," << GRID_H-1 << ")\n";
        bool ok = solver.search(0, MAX_N - 1, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 5: completely blocked ----------
    std::fill(blockMap, blockMap + MAX_N, true); // everything blocked
    blockMap[0] = false;                         // only start open
    blockMap[MAX_N - 1] = false;                 // goal open but isolated
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 5: Isolated start and goal (no connection)\n";
        bool ok = solver.search(0, MAX_N - 1, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    return 0;
}
