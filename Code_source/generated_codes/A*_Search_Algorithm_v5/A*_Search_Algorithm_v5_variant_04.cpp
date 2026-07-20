#include <iostream>
#include <array>
#include <algorithm>

#define MAX_N 36          // 6 x 6 grid
#define MAX_NEI 4         // max neighbours per node
#define INF 1000000

/* LLM input variant 4: signed-extremes */

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
                    // add edge cur -> nxt with varied cost
                    int pos = edgeCnt[cur];
                    neighIdx[cur][pos] = nxt;
                    int cost = 1;
                    if (cur % 3 == 0) cost = -1;             // negative cost for some nodes
                    else if (cur % 5 == 0) cost = 0;         // zero cost for others
                    edgeCost[cur][pos] = cost;
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

    // ---------- Scenario 1: simple open grid ----------
    std::fill(blockMap, blockMap + MAX_N, false);
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 1: Open 6x6 grid, start (0,0) -> goal (4,4)\n";
        bool ok = solver.search(0, 28, path, len, cost); // goal index 28 = (4,4)
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 2: start equals goal ----------
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 2: Start equals Goal (2,3)\n";
        int same = 3 * 6 + 2;               // node index for (2,3)
        bool ok = solver.search(same, same, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 3: blocked goal ----------
    std::fill(blockMap, blockMap + MAX_N, false);
    blockMap[28] = true;                    // block cell (4,4)
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 3: Goal blocked at (4,4)\n";
        bool ok = solver.search(0, 28, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 4: narrow corridor ----------
    std::fill(blockMap, blockMap + MAX_N, false);
    // create a wall leaving a single corridor
    for (int y = 0; y < 6; ++y) {
        if (y != 3) blockMap[y * 6 + 2] = true;   // block column 2 except row 3
        if (y != 2) blockMap[y * 6 + 3] = true;   // block column 3 except row 2
    }
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 4: Narrow corridor, start (0,0) -> goal (5,5)\n";
        bool ok = solver.search(0, 35, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // ---------- Scenario 5: completely blocked ----------
    std::fill(blockMap, blockMap + MAX_N, true); // everything blocked
    blockMap[0] = false;                         // only start open
    blockMap[35] = false;                        // goal open but isolated
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 5: Isolated start and goal (no connection)\n";
        bool ok = solver.search(0, 35, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    return 0;
}
