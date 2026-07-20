#include <iostream>
#include <cstdlib>
#include <ctime>
#include <queue>

/* LLM input variant 6: ordered-structured */

int main() {
    // ---- Parameters ----
    int sizeX = 12;                 // grid width
    int sizeY = 12;                 // grid height
    int maxNodes = sizeX * sizeY;   // maximum possible nodes

    // ---- Deterministic seed (not used for random generation) ----
    std::srand(static_cast<unsigned>(std::time(0)));

    // ---- Grid generation (0 = free, 1 = blocked) ----
    // Structured pattern: a symmetric cross barrier through the center,
    // plus a border of blocked cells except for start and goal.
    int terrain[144];               // stack array: sizeX*sizeY <= 144
    int centerX = sizeX / 2;
    int centerY = sizeY / 2;
    for (int y = 0; y < sizeY; ++y) {
        for (int x = 0; x < sizeX; ++x) {
            int i = y * sizeX + x;
            bool border = (x == 0 || y == 0 || x == sizeX - 1 || y == sizeY - 1);
            bool cross   = (x == centerX || y == centerY);
            terrain[i] = (border || cross) ? 1 : 0;
        }
    }

    // Ensure start and goal are free
    int startX = 0, startY = 0;
    int goalX = sizeX - 1, goalY = sizeY - 1;
    terrain[startY * sizeX + startX] = 0;
    terrain[goalY * sizeX + goalX] = 0;

    // ---- Helper lambdas (inline, but inside main) ----
    auto idx = [&](int x, int y) -> int { return y * sizeX + x; };
    auto manhattan = [&](int x1, int y1, int x2, int y2) -> int {
        int dx = x1 - x2; if (dx < 0) dx = -dx;
        int dy = y1 - y2; if (dy < 0) dy = -dy;
        return dx + dy;
    };

    // ---- Data structures ----
    int openList[144];          // holds indices of nodes in open set
    int openCount = 0;          // number of elements in openList
    int closed[144];            // 0 = not visited, 1 = visited
    float gScore[144];          // cost from start
    float fScore[144];          // estimated total cost
    int cameFrom[144];          // predecessor index

    // Initialize arrays
    for (int i = 0; i < maxNodes; ++i) {
        closed[i] = 0;
        gScore[i] = 1e6f;
        fScore[i] = 1e6f;
        cameFrom[i] = -1;
    }

    // ---- Insert start node into open set ----
    int startIdx = idx(startX, startY);
    int goalIdx  = idx(goalX, goalY);
    gScore[startIdx] = 0.0f;
    fScore[startIdx] = static_cast<float>(manhattan(startX, startY, goalX, goalY));
    openList[openCount++] = startIdx;

    // ---- Main A* loop (iterative, loop‑heavy) ----
    while (openCount > 0) {
        // ---- Find node with smallest fScore ----
        int bestPos = 0;
        for (int i = 1; i < openCount; ++i) {
            if (fScore[openList[i]] < fScore[openList[bestPos]]) {
                bestPos = i;
            }
        }
        int current = openList[bestPos];

        // ---- Goal check ----
        if (current == goalIdx) break;

        // ---- Remove current from open list (swap‑pop) ----
        openList[bestPos] = openList[--openCount];

        // ---- Mark as closed ----
        closed[current] = 1;

        // ---- Expand neighbours (4‑directional) ----
        int cx = current % sizeX;
        int cy = current / sizeX;

        // Direction vectors
        int dX[4] = { 1, -1, 0, 0 };
        int dY[4] = { 0, 0, 1, -1 };

        for (int dir = 0; dir < 4; ++dir) {
            int nx = cx + dX[dir];
            int ny = cy + dY[dir];

            // ---- Bounds check ----
            if (nx < 0 || ny < 0 || nx >= sizeX || ny >= sizeY) continue;

            int nIdx = idx(nx, ny);
            // ---- Skip blocked or already closed ----
            if (terrain[nIdx] == 1 || closed[nIdx] == 1) continue;

            // ---- Tentative gScore (reordered arithmetic) ----
            float tentative = gScore[current] + 1.0f; // uniform cost

            if (tentative < gScore[nIdx]) {
                cameFrom[nIdx] = current;
                gScore[nIdx] = tentative;
                // h = manhattan distance, reorder: f = h + g
                fScore[nIdx] = static_cast<float>(manhattan(nx, ny, goalX, goalY)) + tentative;

                // ---- Insert into open list if not already there ----
                bool inOpen = false;
                for (int i = 0; i < openCount; ++i) {
                    if (openList[i] == nIdx) { inOpen = true; break; }
                }
                if (!inOpen) {
                    openList[openCount++] = nIdx;
                }
            }
        }
    }

    // ---- Reconstruct path ----
    int pathLen = 0;
    int path[144];
    int trace = goalIdx;
    if (cameFrom[goalIdx] != -1 || startIdx == goalIdx) {
        while (trace != -1) {
            path[pathLen++] = trace;
            trace = cameFrom[trace];
        }
    }

    // ---- Output results ----
    std::cout << "Grid size: " << sizeX << " x " << sizeY << "\n";
    std::cout << "Start: (" << startX << "," << startY << ")\n";
    std::cout << "Goal : (" << goalX << "," << goalY << ")\n";
    std::cout << "Path length (nodes): " << pathLen << "\n";
    if (pathLen > 0) {
        std::cout << "Path (reversed order):\n";
        for (int i = 0; i < pathLen; ++i) {
            int p = path[i];
            std::cout << "(" << p % sizeX << "," << p / sizeX << ") ";
        }
        std::cout << "\n";
    } else {
        std::cout << "No path found.\n";
    }

    // ---- Visualise grid with path (optional) ----
    std::cout << "\nGrid view (S=start, G=goal, #=blocked, .=free, *=path):\n";
    for (int y = 0; y < sizeY; ++y) {
        for (int x = 0; x < sizeX; ++x) {
            int id = idx(x, y);
            if (x == startX && y == startY) {
                std::cout << 'S';
            } else if (x == goalX && y == goalY) {
                std::cout << 'G';
            } else {
                bool onPath = false;
                for (int p = 0; p < pathLen; ++p) if (path[p] == id) { onPath = true; break; }
                if (onPath) std::cout << '*';
                else if (terrain[id] == 1) std::cout << '#';
                else std::cout << '.';
            }
        }
        std::cout << '\n';
    }

    return 0;
}
