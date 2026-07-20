/* LLM input variant 6: ordered-structured */
/*  A* Search – version #6  */
#include <iostream>
#include <cstdlib>
#include <ctime>

#define GRID_SIZE 50               // size of the square grid
#define MAX_NODES (GRID_SIZE*GRID_SIZE)
#define INF 1000000                // large number for initialization
#define OBSTACLE_PROB 30           // percentage chance a cell is blocked

/* -------------------------------------------------------------
   Helper: compute Manhattan distance (heuristic)
   ------------------------------------------------------------- */
int manhattan(int x1, int y1, int x2, int y2)
{
    int dx = x1 - x2;
    if (dx < 0) dx = -dx;          // abs without std::abs
    int dy = y1 - y2;
    if (dy < 0) dy = -dy;
    int h = dx + dy;               // simple sum
    return h;
}

/* -------------------------------------------------------------
   Helper: reconstruct and print the path
   ------------------------------------------------------------- */
void print_path(int cameFrom[], int startIdx, int goalIdx)
{
    int path[MAX_NODES];
    int length = 0;
    int cur = goalIdx;

    while (cur != -1 && cur != startIdx)
    {
        path[length++] = cur;
        cur = cameFrom[cur];
    }
    if (cur == startIdx) path[length++] = startIdx;

    std::cout << "Path length (nodes): " << length << "\n";
    std::cout << "Path (x y): ";
    for (int i = length - 1; i >= 0; --i)
    {
        int y = path[i] / GRID_SIZE;
        int x = path[i] % GRID_SIZE;
        std::cout << "(" << x << "," << y << ") ";
    }
    std::cout << "\n";
}

/* -------------------------------------------------------------
   Helper: find index of node with lowest fScore among open set
   ------------------------------------------------------------- */
int lowest_f(int openList[], int openCount, float fScore[])
{
    int bestIdx = -1;
    float bestVal = 1e9f;          // huge initial value
    for (int i = 0; i < openCount; ++i)
    {
        int node = openList[i];
        float curF = fScore[node];
        if (curF < bestVal)
        {
            bestVal = curF;
            bestIdx = i;
        }
    }
    return bestIdx;
}

/* -------------------------------------------------------------
   Main driver – uses a deterministic, highly structured grid and runs A*
   ------------------------------------------------------------- */
int main()
{
    /* ---------------------------------------------------------
       Grid representation: 0 = free, 1 = obstacle
       Structured pattern: obstacles on even-indexed diagonal cells
       --------------------------------------------------------- */
    int grid[MAX_NODES];
    for (int i = 0; i < MAX_NODES; ++i)
    {
        int row = i / GRID_SIZE;
        int col = i % GRID_SIZE;
        // Place an obstacle on cells where row == col and row is even
        if (row == col && (row % 2 == 0))
            grid[i] = 1;
        else
            grid[i] = 0;
    }

    /* ---------------------------------------------------------
       Deterministic start (top‑left) and goal (bottom‑right) that are free
       --------------------------------------------------------- */
    int startIdx = 0;
    while (grid[startIdx] == 1 && startIdx < MAX_NODES) ++startIdx;

    int goalIdx = MAX_NODES - 1;
    while (grid[goalIdx] == 1 && goalIdx >= 0) --goalIdx;

    int startX = startIdx % GRID_SIZE;
    int startY = startIdx / GRID_SIZE;
    int goalX  = goalIdx  % GRID_SIZE;
    int goalY  = goalIdx  / GRID_SIZE;

    std::cout << "Start: (" << startX << "," << startY << ")\n";
    std::cout << "Goal : (" << goalX  << "," << goalY  << ")\n";

    /* ---------------------------------------------------------
       A* data structures (all on stack)
       --------------------------------------------------------- */
    int  cameFrom[MAX_NODES];
    int  gScore[MAX_NODES];
    float fScore[MAX_NODES];
    int  openList[MAX_NODES];
    int  openCount = 0;
    int  closed[MAX_NODES];        // 0 = not closed, 1 = closed

    /* ---------------------------------------------------------
       Initialisation
       --------------------------------------------------------- */
    for (int i = 0; i < MAX_NODES; ++i)
    {
        cameFrom[i] = -1;
        gScore[i] = INF;
        fScore[i] = 1e9f;
        closed[i] = 0;
    }

    gScore[startIdx] = 0;
    int h0 = manhattan(startX, startY, goalX, goalY);
    fScore[startIdx] = static_cast<float>(h0);
    openList[openCount++] = startIdx;

    /* ---------------------------------------------------------
       Main A* loop – completely iterative, no recursion
       --------------------------------------------------------- */
    while (openCount > 0)
    {
        int bestPos = lowest_f(openList, openCount, fScore);
        int current = openList[bestPos];

        if (current == goalIdx)
        {
            std::cout << "Goal reached!\n";
            print_path(cameFrom, startIdx, goalIdx);
            return 0;
        }

        /* ---- remove current from open list (swap‑pop) ---- */
        openList[bestPos] = openList[--openCount];

        closed[current] = 1;

        int curX = current % GRID_SIZE;
        int curY = current / GRID_SIZE;

        /* ---- explore 4‑directional neighbours ---- */
        for (int dir = 0; dir < 4; ++dir)
        {
            int nx = curX;
            int ny = curY;

            if (dir == 0) nx = curX - 1;          // left
            if (dir == 1) nx = curX + 1;          // right
            if (dir == 2) ny = curY - 1;          // up
            if (dir == 3) ny = curY + 1;          // down

            /* ---- bounds check ---- */
            if (nx < 0 || nx >= GRID_SIZE || ny < 0 || ny >= GRID_SIZE)
                continue;

            int neighborIdx = ny * GRID_SIZE + nx;

            /* ---- skip obstacles and closed nodes ---- */
            if (grid[neighborIdx] == 1 || closed[neighborIdx] == 1)
                continue;

            /* ---- tentative gScore computation (expanded steps) ---- */
            int stepA = gScore[current];
            int stepB = stepA + 1;                // uniform cost = 1
            int tentative_g = stepB;

            if (tentative_g < gScore[neighborIdx])
            {
                cameFrom[neighborIdx] = current;
                gScore[neighborIdx] = tentative_g;

                int hx = manhattan(nx, ny, goalX, goalY);
                float stepC = static_cast<float>(tentative_g);
                float stepD = static_cast<float>(hx);
                fScore[neighborIdx] = stepC + stepD;

                /* ---- add neighbour to open list if not already there ---- */
                bool alreadyOpen = false;
                for (int i = 0; i < openCount; ++i)
                {
                    if (openList[i] == neighborIdx)
                    {
                        alreadyOpen = true;
                        break;
                    }
                }
                if (!alreadyOpen)
                    openList[openCount++] = neighborIdx;
            }
        }
    }

    std::cout << "No path found.\n";
    return 0;
}
