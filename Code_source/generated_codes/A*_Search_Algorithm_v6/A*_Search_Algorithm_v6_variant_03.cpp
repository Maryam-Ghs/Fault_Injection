/* LLM input variant 3: zeros-and-ones */
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
   Main driver – deterministic grid emphasizing zeros and ones
   ------------------------------------------------------------- */
int main()
{
    /* ---------------------------------------------------------
       Grid representation: 0 = free, 1 = obstacle
       --------------------------------------------------------- */
    int grid[MAX_NODES];
    // Fill all cells with 0 (free)
    for (int i = 0; i < MAX_NODES; ++i)
        grid[i] = 0;

    // Place a few obstacles (1s) at specific positions
    // These positions are chosen to be sparse
    int obstacle_positions[] = {
        1,                                   // (1,0)
        GRID_SIZE,                           // (0,1)
        GRID_SIZE * 2 + 2,                  // (2,2)
        MAX_NODES - GRID_SIZE - 1,           // (48,49)
        MAX_NODES - 2                        // (48,49) second last cell as obstacle
    };
    int obstacle_count = sizeof(obstacle_positions) / sizeof(obstacle_positions[0]);
    for (int i = 0; i < obstacle_count; ++i)
        grid[obstacle_positions[i]] = 1;

    /* ---------------------------------------------------------
       Deterministic start (top‑left) and goal (bottom‑right)
       --------------------------------------------------------- */
    int startIdx = 0;                     // (0,0)
    int goalIdx = MAX_NODES - 1;          // (GRID_SIZE-1, GRID_SIZE-1)

    // Ensure start and goal are not obstacles (override if needed)
    grid[startIdx] = 0;
    grid[goalIdx] = 0;

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
