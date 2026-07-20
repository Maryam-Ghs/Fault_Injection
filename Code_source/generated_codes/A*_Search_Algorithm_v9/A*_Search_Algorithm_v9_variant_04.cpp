// LLM input variant 4: signed-extremes
// A* Search implementation – version #9
// -------------------------------------------------
// Generates a small grid internally, runs A* and prints the found path.
// -------------------------------------------------

#include <iostream>
#include <algorithm>

struct Tile {
    int xPos;
    int yPos;
    int gCost;          // cost from start
    float fCost;        // g + h
    int parentIdx;      // index in closed list of predecessor
};

int gridRows = 5;
int gridCols = 5;

// 0 = free, 1 = obstacle, mixed signed values for testing
int obstacleMap[5][5] = {
    { 0, -1,  0,  2, 0},
    { 0,  1,  1, -1, 0},
    { 2,  0, -1,  1, 0},
    { 0,  1,  0,  0, -1},
    { 0,  0,  2,  0, 0}
};

int startX = 0, startY = 0;   // zero values
int goalX  = 4, goalY  = 4;   // positive values

// -------------------------------------------------
// Helper: Manhattan distance (heuristic)
float manhattan(int ax, int ay, int bx, int by) {
    int dx = ax - bx; if (dx < 0) dx = -dx;
    int dy = ay - by; if (dy < 0) dy = -dy;
    return (float)(dx + dy);
}

// -------------------------------------------------
// Helper: check if a position is inside the grid
bool inside(int ax, int ay) {
    return (ax >= 0 && ax < gridRows && ay >= 0 && ay < gridCols);
}

// -------------------------------------------------
// Helper: test if a tile is already in closed list
bool inClosed(const Tile closed[], int closedCnt, int ax, int ay) {
    for (int i = 0; i < closedCnt; ++i) {
        if (closed[i].xPos == ax && closed[i].yPos == ay) return true;
    }
    return false;
}

// -------------------------------------------------
// Helper: retrieve index of tile in open list (or -1)
int indexInOpen(const Tile open[], int openCnt, int ax, int ay) {
    for (int i = 0; i < openCnt; ++i) {
        if (open[i].xPos == ax && open[i].yPos == ay) return i;
    }
    return -1;
}

// -------------------------------------------------
// Helper: pick the open node with lowest fCost
int extractBest(Tile open[], int &openCnt) {
    int bestIdx = 0;
    for (int i = 1; i < openCnt; ++i) {
        if (open[i].fCost < open[bestIdx].fCost) bestIdx = i;
    }
    Tile best = open[bestIdx];
    // shift remaining elements left
    for (int i = bestIdx; i < openCnt - 1; ++i) open[i] = open[i + 1];
    --openCnt;
    // place best at the end of closed list later
    open[openCnt] = best;   // temporary hold
    return openCnt;         // index where best is stored
}

// -------------------------------------------------
// Helper: reconstruct path from closed list
void printPath(const Tile closed[], int goalIdx) {
    int pathLen = 0;
    int rev[100][2]; // store reversed coordinates

    int cur = goalIdx;
    while (cur != -1) {
        rev[pathLen][0] = closed[cur].xPos;
        rev[pathLen][1] = closed[cur].yPos;
        ++pathLen;
        cur = closed[cur].parentIdx;
    }

    std::cout << "Path (from start to goal):\n";
    for (int i = pathLen - 1; i >= 0; --i) {
        std::cout << "(" << rev[i][0] << "," << rev[i][1] << ")";
        if (i) std::cout << " -> ";
    }
    std::cout << "\n";
}

// -------------------------------------------------
int main() {
    // maximum possible nodes = rows * cols
    Tile openSet[25];
    Tile closedSet[25];
    int openCnt = 0, closedCnt = 0;

    // seed the open list with the start node
    Tile startTile;
    startTile.xPos = startX;
    startTile.yPos = startY;
    startTile.gCost = 0;
    startTile.fCost = manhattan(startX, startY, goalX, goalY);
    startTile.parentIdx = -1;
    openSet[openCnt++] = startTile;

    // direction vectors (up, down, left, right)
    int dX[4] = { -1, 1, 0, 0 };
    int dY[4] = { 0, 0, -1, 1 };

    int goalIdx = -1;

    while (openCnt > 0) {
        // pick node with smallest fCost
        int bestPos = extractBest(openSet, openCnt);
        Tile current = openSet[openCnt]; // retrieved node
        // move to closed list
        closedSet[closedCnt++] = current;

        // reached goal?
        if (current.xPos == goalX && current.yPos == goalY) {
            goalIdx = closedCnt - 1;
            break;
        }

        // examine neighbours
        for (int dir = 0; dir < 4; ++dir) {
            int nx = current.xPos + dX[dir];
            int ny = current.yPos + dY[dir];

            // skip invalid or blocked cells
            if (!inside(nx, ny)) continue;
            if (obstacleMap[nx][ny] == 1) continue;
            if (inClosed(closedSet, closedCnt, nx, ny)) continue;

            int tentativeG = current.gCost + 1; // uniform cost

            int posInOpen = indexInOpen(openSet, openCnt, nx, ny);
            if (posInOpen == -1) {
                // create new node
                Tile neighbor;
                neighbor.xPos = nx;
                neighbor.yPos = ny;
                neighbor.gCost = tentativeG;
                neighbor.fCost = (float)tentativeG + manhattan(nx, ny, goalX, goalY);
                neighbor.parentIdx = closedCnt - 1;
                openSet[openCnt++] = neighbor;
            } else {
                // better path?
                if (tentativeG < openSet[posInOpen].gCost) {
                    openSet[posInOpen].gCost = tentativeG;
                    openSet[posInOpen].fCost = (float)tentativeG + manhattan(nx, ny, goalX, goalY);
                    openSet[posInOpen].parentIdx = closedCnt - 1;
                }
            }
        }
    }

    if (goalIdx != -1) {
        std::cout << "Goal reached!\n";
        printPath(closedSet, goalIdx);
    } else {
        std::cout << "No path found.\n";
    }

    return 0;
}
