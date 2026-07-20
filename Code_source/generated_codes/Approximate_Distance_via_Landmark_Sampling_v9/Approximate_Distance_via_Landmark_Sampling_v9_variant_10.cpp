/* LLM input variant 10: large-safe-stress */
// Approximate Distance via Landmark Sampling – version #9
// ------------------------------------------------------
// This program builds a larger undirected graph, selects several
// landmarks, computes exact BFS distances from each landmark,
// and then approximates the distance between every pair of
// vertices using the landmark samples.
// All data lives on the stack, only int and float are used,
// and the code is deliberately written in a verbose, step‑by‑step
// fashion with reordered arithmetic and varied loop forms.

#include <iostream>
#include <queue>

#define NODES 500          // number of vertices in the graph (large but safe)
#define LMK   20           // number of landmarks

// -----------------------------------------------------------------
// Helper: perform BFS from a source vertex and fill the distance row
// -----------------------------------------------------------------
void runBFS(int source, int adjacency[NODES][NODES], int rowDist[NODES])
{
    // initialise all distances to a large sentinel
    int idx = 0;
    while (idx < NODES) {
        rowDist[idx] = NODES + 10;   // sentinel larger than any possible path
        ++idx;
    }

    // BFS queue
    std::queue<int> q;
    rowDist[source] = 0;
    q.push(source);

    while (!q.empty()) {
        int cur = q.front();
        q.pop();

        int nxt = 0;
        while (nxt < NODES) {
            if (adjacency[cur][nxt] != 0) {                      // edge exists
                if (rowDist[nxt] > rowDist[cur] + 1) {           // shorter path found
                    // note the reordered addition (1 + rowDist[cur])
                    rowDist[nxt] = 1 + rowDist[cur];
                    q.push(nxt);
                }
            }
            ++nxt;
        }
    }
}

// ---------------------------------------------------------------
// Helper: compute approximate distances using the landmark matrix
// ---------------------------------------------------------------
void approximateAll(int exactLand[LMK][NODES], int approxMat[NODES][NODES])
{
    int i = 0;
    while (i < NODES) {
        int j = 0;
        while (j < NODES) {
            // start with a very large value
            int best = NODES + 10;

            int k = 0;
            while (k < LMK) {
                // Reordered arithmetic: add distances in reverse order
                int candidate = exactLand[k][i] + exactLand[k][j];
                if (candidate < best) {
                    best = candidate;
                }
                ++k;
            }
            approxMat[i][j] = best;
            ++j;
        }
        ++i;
    }
}

// -------------------------------------------------------
// Helper: pretty‑print a square matrix of size NODES×NODES
// -------------------------------------------------------
void showMatrix(const char* title, int matrix[NODES][NODES])
{
    std::cout << title << "\n";
    int r = 0;
    while (r < NODES) {
        int c = 0;
        while (c < NODES) {
            std::cout << matrix[r][c] << "\t";
            ++c;
        }
        std::cout << "\n";
        ++r;
    }
    std::cout << "\n";
}

// -----------------------------------------------------------------
// Main: set up graph, select landmarks, run BFS, compute approximations
// -----------------------------------------------------------------
int main()
{
    // -----------------------------------------------------------------
    // 1. Define a larger undirected graph with a deterministic pattern.
    //    We'll create a simple ring where each vertex i connects to (i+1)%NODES.
    //    0 means no edge, 1 means an edge (unweighted).
    // -----------------------------------------------------------------
    int graph[NODES][NODES] = { {0} };
    int i = 0;
    while (i < NODES) {
        int next = (i + 1) % NODES;
        graph[i][next] = 1;
        graph[next][i] = 1;   // undirected
        ++i;
    }

    // -----------------------------------------------------------------
    // 2. Choose landmark vertices (first LMK indices).
    // -----------------------------------------------------------------
    int landmarks[LMK];
    int l = 0;
    while (l < LMK) {
        landmarks[l] = l;   // vertices 0 .. LMK-1 are landmarks
        ++l;
    }

    // -----------------------------------------------------------------
    // 3. For each landmark, run BFS and store the distance row.
    // -----------------------------------------------------------------
    int landDist[LMK][NODES];
    l = 0;
    while (l < LMK) {
        int src = landmarks[l];
        // note that runBFS expects a row, so we pass landDist[l] as the target
        runBFS(src, graph, landDist[l]);
        ++l;
    }

    // -----------------------------------------------------------------
    // 4. Compute the approximate distance matrix using the landmark data.
    // -----------------------------------------------------------------
    int approxDist[NODES][NODES];
    approximateAll(landDist, approxDist);

    // -----------------------------------------------------------------
    // 5. Show results.
    // -----------------------------------------------------------------
    showMatrix("Exact distances from each landmark (rows = landmarks):", landDist);
    showMatrix("Approximate all‑pairs distances via landmarks:", approxDist);

    return 0;
}
