#include <iostream>
#include <queue>
#include <cstring>

#define MAXV 6               // maximum number of vertices in any test graph
#define INF  1000            // sufficiently large for our purposes

/* LLM input variant 9: medium-deterministic-random */

// ------------------------------------------------------------
// Compute the vertex‑connectivity between two vertices using
// a simple Edmonds‑Karp max‑flow on a vertex‑split graph.
// ------------------------------------------------------------
int vertexCut(int n, int adj[MAXV][MAXV], int s, int t)
{
    if (s == t) return 0;                     // same vertex -> zero cut

    const int total = 2 * MAXV;               // split each vertex into in/out
    int cap[2 * MAXV][2 * MAXV];
    memset(cap, 0, sizeof(cap));

    // ---- build split‑graph capacities ---------------------------------
    for (int i = 0; i < n; ++i)
    {
        int in  = i;           // i_in
        int out = i + MAXV;    // i_out

        // capacity of vertex i (except s and t)
        cap[in][out] = (i == s || i == t) ? INF : 1;
    }

    // edges become infinite capacity arcs between out‑nodes and in‑nodes
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (adj[i][j])
            {
                int i_out = i + MAXV;
                int j_in  = j;
                cap[i_out][j_in] = INF;
            }

    // ---- Edmonds‑Karp --------------------------------------------------
    int flow = 0;
    int parent[2 * MAXV];

    while (true)
    {
        // BFS to find augmenting path
        for (int i = 0; i < total; ++i) parent[i] = -1;
        std::queue<int> q;
        q.push(s);
        parent[s] = s;

        while (!q.empty() && parent[t] == -1)
        {
            int u = q.front(); q.pop();
            for (int v = 0; v < total; ++v)
                if (parent[v] == -1 && cap[u][v] > 0)
                {
                    parent[v] = u;
                    q.push(v);
                }
        }

        if (parent[t] == -1) break;          // no augmenting path

        // find bottleneck (will be 1 or INF)
        int aug = INF;
        for (int v = t; v != s; v = parent[v])
        {
            int u = parent[v];
            if (cap[u][v] < aug) aug = cap[u][v];
        }

        // augment flow and update residual capacities
        for (int v = t; v != s; v = parent[v])
        {
            int u = parent[v];
            cap[u][v] -= aug;
            cap[v][u] += aug;
        }
        flow += aug;
        if (flow >= INF) break;               // cannot exceed INF for vertex cuts
    }

    return flow;
}

// ------------------------------------------------------------
// Compute all‑pairs vertex connectivity for a given graph.
// ------------------------------------------------------------
void allPairsConn(int n, int adj[MAXV][MAXV], int result[MAXV][MAXV])
{
    for (int a = 0; a < n; ++a)
        for (int b = 0; b < n; ++b)
            result[a][b] = vertexCut(n, adj, a, b);
}

// ------------------------------------------------------------
// Helper to print a matrix.
// ------------------------------------------------------------
void printMat(int n, int mat[MAXV][MAXV])
{
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            std::cout << mat[i][j];
            if (j + 1 < n) std::cout << ' ';
        }
        std::cout << '\n';
    }
}

// deterministic pseudo‑random edge generator (0/1) based on indices
int pseudoEdge(int i, int j, int gidx)
{
    // simple deterministic formula producing a 0/1 pattern
    return ((i * 31 + j * 17 + gidx * 13) % 2);
}

// ------------------------------------------------------------
// Main: generate deterministic pseudo‑random inputs, compute, and print.
// ------------------------------------------------------------
int main()
{
    const int graphCount = 5;
    int ns[graphCount] = {2, 3, 4, 5, 6};

    // storage for adjacency matrices of each graph
    int adj[graphCount][MAXV][MAXV];
    // initialize all entries to 0
    for (int g = 0; g < graphCount; ++g)
        for (int i = 0; i < MAXV; ++i)
            for (int j = 0; j < MAXV; ++j)
                adj[g][i][j] = 0;

    // fill each matrix with a deterministic pseudo‑random pattern
    for (int g = 0; g < graphCount; ++g)
    {
        int n = ns[g];
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                int val = pseudoEdge(i, j, g);
                adj[g][i][j] = val;
                adj[g][j][i] = val; // ensure symmetry (undirected graph)
            }
        }
    }

    // Array of graphs
    struct Graph { int n; int (*adj)[MAXV]; };
    Graph graphs[graphCount];
    for (int g = 0; g < graphCount; ++g)
    {
        graphs[g].n = ns[g];
        graphs[g].adj = adj[g];
    }

    // ---- process each graph -------------------------------------------
    for (int idx = 0; idx < graphCount; ++idx)
    {
        int n = graphs[idx].n;
        int (*adjPtr)[MAXV] = graphs[idx].adj;
        int conn[MAXV][MAXV];

        allPairsConn(n, adjPtr, conn);

        std::cout << "Graph " << idx + 1 << " (" << n << " vertices):\n";
        printMat(n, conn);
        std::cout << '\n';
    }

    return 0;
}
