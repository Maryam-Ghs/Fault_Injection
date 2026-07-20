/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <array>
#include <algorithm>

#define GRID_W 2
#define GRID_H 2
#define MAX_N (GRID_W * GRID_H)   // minimal 2x2 grid
#define MAX_NEI 4
#define INF 1000000

class AStarPathfinder {
    int edgeCnt[MAX_N];
    int neighIdx[MAX_N][MAX_NEI];
    int edgeCost[MAX_N][MAX_NEI];

    int gScore[MAX_N];
    int fScore[MAX_N];
    int cameFrom[MAX_N];
    bool openSet[MAX_N];
    bool closedSet[MAX_N];

    int heuristic(int a, int b) {
        int ax = a % GRID_W, ay = a / GRID_W;
        int bx = b % GRID_W, by = b / GRID_W;
        return (ax > bx ? ax - bx : bx - ax) + (ay > by ? ay - by : by - ay);
    }

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
    void buildGrid(const bool obstacles[MAX_N]) {
        for (int i = 0; i < MAX_N; ++i) edgeCnt[i] = 0;

        const int dx[4] = {1, -1, 0, 0};
        const int dy[4] = {0, 0, 1, -1};

        for (int y = 0; y < GRID_H; ++y) {
            for (int x = 0; x < GRID_W; ++x) {
                int cur = y * GRID_W + x;
                if (obstacles[cur]) continue;

                for (int d = 0; d < 4; ++d) {
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) continue;
                    int nxt = ny * GRID_W + nx;
                    if (obstacles[nxt]) continue;
                    int pos = edgeCnt[cur];
                    neighIdx[cur][pos] = nxt;
                    edgeCost[cur][pos] = 1;
                    ++edgeCnt[cur];
                }
            }
        }
    }

    bool search(int startIdx, int goalIdx, int outPath[MAX_N], int &outLen, int &outCost) {
        resetArrays();

        gScore[startIdx] = 0;
        fScore[startIdx] = heuristic(startIdx, goalIdx);
        openSet[startIdx] = true;

        while (true) {
            int bestNode = -1;
            int bestF = INF;
            for (int i = 0; i < MAX_N; ++i) {
                if (openSet[i] && fScore[i] < bestF) {
                    bestF = fScore[i];
                    bestNode = i;
                }
            }

            if (bestNode == -1) break;

            if (bestNode == goalIdx) {
                outLen = 0;
                int trace = goalIdx;
                while (trace != -1) {
                    outPath[outLen++] = trace;
                    trace = cameFrom[trace];
                }
                for (int i = 0; i < outLen / 2; ++i)
                    std::swap(outPath[i], outPath[outLen - 1 - i]);
                outCost = gScore[goalIdx];
                return true;
            }

            openSet[bestNode] = false;
            closedSet[bestNode] = true;

            int neighbours = edgeCnt[bestNode];
            for (int n = 0; n < neighbours; ++n) {
                int nxt = neighIdx[bestNode][n];
                if (closedSet[nxt]) continue;

                int tentativeG = gScore[bestNode] + edgeCost[bestNode][n];
                if (!openSet[nxt]) openSet[nxt] = true;
                if (tentativeG >= gScore[nxt]) continue;

                cameFrom[nxt] = bestNode;
                gScore[nxt] = tentativeG;
                fScore[nxt] = gScore[nxt] + heuristic(nxt, goalIdx);
            }
        }

        outLen = 0;
        outCost = INF;
        return false;
    }
};

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

int main() {
    AStarPathfinder solver;
    bool blockMap[MAX_N] = {false};

    // Scenario 1: simple open grid
    std::fill(blockMap, blockMap + MAX_N, false);
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 1: Open 2x2 grid, start (0,0) -> goal (1,1)\n";
        bool ok = solver.search(0, 3, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // Scenario 2: start equals goal
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 2: Start equals Goal (1,0)\n";
        int same = 0 * GRID_W + 1; // node (1,0)
        bool ok = solver.search(same, same, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // Scenario 3: blocked goal
    std::fill(blockMap, blockMap + MAX_N, false);
    blockMap[3] = true; // block cell (1,1)
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 3: Goal blocked at (1,1)\n";
        bool ok = solver.search(0, 3, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    // Scenario 4: isolated start and goal
    std::fill(blockMap, blockMap + MAX_N, true);
    blockMap[0] = false; // start open
    blockMap[3] = false; // goal open but no connection
    solver.buildGrid(blockMap);
    {
        int path[MAX_N];
        int len = 0, cost = 0;
        std::cout << "Scenario 4: Isolated start and goal (no connection)\n";
        bool ok = solver.search(0, 3, path, len, cost);
        if (ok) showPath(path, len, cost);
        else std::cout << "No path found.\n";
        std::cout << "--------------------------\n";
    }

    return 0;
}
