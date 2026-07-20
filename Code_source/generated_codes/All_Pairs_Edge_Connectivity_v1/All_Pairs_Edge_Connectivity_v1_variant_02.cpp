/* LLM input variant 2: small-diverse */
#include <iostream>
#include <queue>
#include <algorithm>

#define MAXV 8                 // maximum number of vertices (stack allocation)

// ---------------------------------------------------------------------------
// Helper: Breadth‑first search that also records the augmenting path.
// Returns true if sink is reachable from source in the residual network.
// ---------------------------------------------------------------------------
bool bfs(int src, int snk, int V, int ResCap[MAXV][MAXV], int Prev[])
{
    // initialise visited array (0 = unvisited, 1 = visited)
    int Vis[MAXV];
    int i = 0;
    while (i < V) { Vis[i] = 0; ++i; }

    std::queue<int> Q;
    Q.push(src);
    Vis[src] = 1;
    Prev[src] = -1;

    while (!Q.empty())
    {
        int cur = Q.front(); Q.pop();

        // ---- manual unrolling of the neighbour loop (step = 4) ----
        int j = 0;
        while (j + 3 < V)
        {
            if (!Vis[j] && ResCap[cur][j] > 0) { Q.push(j); Vis[j] = 1; Prev[j] = cur; }
            if (!Vis[j+1] && ResCap[cur][j+1] > 0) { Q.push(j+1); Vis[j+1] = 1; Prev[j+1] = cur; }
            if (!Vis[j+2] && ResCap[cur][j+2] > 0) { Q.push(j+2); Vis[j+2] = 1; Prev[j+2] = cur; }
            if (!Vis[j+3] && ResCap[cur][j+3] > 0) { Q.push(j+3); Vis[j+3] = 1; Prev[j+3] = cur; }
            j += 4;
        }
        while (j < V)          // tail part
        {
            if (!Vis[j] && ResCap[cur][j] > 0) { Q.push(j); Vis[j] = 1; Prev[j] = cur; }
            ++j;
        }
        // ------------------------------------------------------------
    }
    return Vis[snk];
}

// ---------------------------------------------------------------------------
// Helper: Compute max‑flow (hence min‑cut) between two vertices using Edmonds‑Karp.
// ---------------------------------------------------------------------------
int maxflow(int src, int snk, int V, int Cap[MAXV][MAXV])
{
    // residual capacity matrix (copy of original capacities)
    int ResCap[MAXV][MAXV];
    int i = 0;
    while (i < V) { int k = 0; while (k < V) { ResCap[i][k] = Cap[i][k]; ++k; } ++i; }

    int Prev[MAXV];
    int flow = 0;

    while (bfs(src, snk, V, ResCap, Prev))
    {
        // find bottleneck capacity on the discovered path
        int bottleneck = 1e9;
        int v = snk;
        while (Prev[v] != -1)
        {
            int u = Prev[v];
            // fused min expression
            bottleneck = (ResCap[u][v] < bottleneck) ? ResCap[u][v] : bottleneck;
            v = u;
        }

        // augment flow and update residual capacities (unrolled for 4 edges)
        v = snk;
        while (Prev[v] != -1)
        {
            int u = Prev[v];
            ResCap[u][v] -= bottleneck;
            ResCap[v][u] += bottleneck;
            v = u;
        }
        flow += bottleneck;
    }
    return flow;
}

// ---------------------------------------------------------------------------
// Helper: Print the all‑pairs edge‑connectivity matrix.
// ---------------------------------------------------------------------------
void showResult(int V, int Conn[MAXV][MAXV])
{
    std::cout << "All‑Pairs Edge Connectivity (min‑cut values):\n";
    for (int r = 0; r < V; ++r)
    {
        for (int c = 0; c < V; ++c)
        {
            // print a dash for self‑pair
            if (r == c) std::cout << "-\t";
            else std::cout << Conn[r][c] << "\t";
        }
        std::cout << "\n";
    }
}

// ---------------------------------------------------------------------------
// Main: generate a challenging test graph and compute pairwise connectivity.
// ---------------------------------------------------------------------------
int main()
{
    // ---- generate a graph with a variety of edge‑cases ----
    // V = 5 (indices 0‑4); vertex 4 will be isolated (edge‑case).
    int Vcnt = 5;
    int AdjCap[MAXV][MAXV];          // capacity matrix on the stack

    // initialise all capacities to zero
    int i = 0;
    while (i < Vcnt) { int j = 0; while (j < Vcnt) { AdjCap[i][j] = 0; ++j; } ++i; }

    // add parallel edges by summing capacities (0‑1 gets 4+2 = 6)
    AdjCap[0][1] = 6; AdjCap[1][0] = 6;   // parallel edges, high capacity
    AdjCap[0][2] = 2; AdjCap[2][0] = 2;   // low capacity
    AdjCap[1][2] = 3; AdjCap[2][1] = 3;   // medium capacity
    AdjCap[1][3] = 0; AdjCap[3][1] = 0;   // zero‑capacity edge (edge‑case)
    AdjCap[2][3] = 5; AdjCap[3][2] = 5;   // higher capacity
    // self‑loops (should be ignored by flow algorithm)
    AdjCap[2][2] = 7; AdjCap[3][3] = 6;

    // ---- compute all‑pairs edge connectivity using repeated max‑flow ----
    int ConnMat[MAXV][MAXV];
    i = 0;
    while (i < Vcnt)
    {
        int j = 0;
        while (j < Vcnt)
        {
            if (i == j) { ConnMat[i][j] = 0; }
            else
            {
                // compute min‑cut i→j (undirected graph, symmetric result)
                ConnMat[i][j] = maxflow(i, j, Vcnt, AdjCap);
            }
            ++j;
        }
        ++i;
    }

    // ---- output the matrix ----
    showResult(Vcnt, ConnMat);
    return 0;
}
