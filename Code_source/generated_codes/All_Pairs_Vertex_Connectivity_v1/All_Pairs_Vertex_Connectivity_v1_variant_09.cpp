#include <iostream>
#include <queue>

#define MAXN 6                // maximum number of original vertices
#define INF 1000              // a value larger than any possible flow

/* LLM input variant 9: medium-deterministic-random */

// ------------------------------------------------------------
// Helper: builds the flow network for a given source‑target pair
// ------------------------------------------------------------
void buildNetwork(int n,
                  int origAdj[MAXN][MAXN],
                  int src,
                  int dst,
                  int cap[2*MAXN][2*MAXN])
{
    int total = 2 * n;

    // clear capacities
    for (int i = 0; i < total; ++i)
        for (int j = 0; j < total; ++j)
            cap[i][j] = 0;

    // split each vertex into in/out nodes
    for (int v = 0; v < n; ++v) {
        int vin = v * 2;
        int vout = vin + 1;
        // source and sink get infinite internal capacity
        if (v == src || v == dst) {
            cap[vin][vout] = INF;
        } else {
            cap[vin][vout] = 1;          // ordinary vertex capacity
        }
    }

    // add edges for each undirected original edge
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            if (origAdj[u][v] == 0) continue;
            int uout = u * 2 + 1;
            int vin  = v * 2;
            cap[uout][vin] = INF;        // unlimited edge capacity
        }
    }
}

// ------------------------------------------------------------
// Helper: BFS on residual graph, stores parent links
// ------------------------------------------------------------
bool bfs(int total,
         int res[2*MAXN][2*MAXN],
         int s,
         int t,
         int parent[2*MAXN])
{
    bool visited[2*MAXN] = { false };
    std::queue<int> q;

    q.push(s);
    visited[s] = true;
    parent[s] = -1;

    while (!q.empty()) {
        int cur = q.front(); q.pop();

        for (int nxt = 0; nxt < total; ++nxt) {
            if (!visited[nxt] && res[cur][nxt] > 0) {
                q.push(nxt);
                parent[nxt] = cur;
                visited[nxt] = true;
                if (nxt == t) return true;
            }
        }
    }
    return false;
}

// ------------------------------------------------------------
// Helper: Edmonds‑Karp max‑flow (which equals vertex connectivity)
// ------------------------------------------------------------
int maxFlow(int total,
            int s,
            int t,
            int cap[2*MAXN][2*MAXN])
{
    int residual[2*MAXN][2*MAXN];
    // copy capacities into residual matrix
    for (int i = 0; i < total; ++i)
        for (int j = 0; j < total; ++j)
            residual[i][j] = cap[i][j];

    int parent[2*MAXN];
    int flow = 0;

    while (bfs(total, residual, s, t, parent)) {
        // find bottleneck
        int inc = INF;
        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            if (residual[u][v] < inc) inc = residual[u][v];
        }
        // augment path
        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            residual[u][v] -= inc;
            residual[v][u] += inc;
        }
        flow += inc;
    }
    return flow;
}

// ------------------------------------------------------------
// Main: generate a graph with many edge‑case patterns,
// compute all‑pairs vertex connectivity, and print it.
// ------------------------------------------------------------
int main()
{
    // number of vertices (choose a small number to stay on stack)
    int n = 6;

    // adjacency matrix of the original undirected graph
    // deterministic pseudo‑random pattern:
    // 0: connects to 1,2,4
    // 1: connects to 0,3,5
    // 2: connects to 0,3
    // 3: connects to 1,2,4,5
    // 4: connects to 0,3
    // 5: connects to 1,3
    int graph[MAXN][MAXN] = {
        {0,1,1,0,1,0},
        {1,0,0,1,0,1},
        {1,0,0,1,0,0},
        {0,1,1,0,1,1},
        {1,0,0,1,0,0},
        {0,1,0,1,0,0}
    };

    // matrix to hold results
    int result[MAXN][MAXN];

    // compute connectivity for each ordered pair (i,j)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                result[i][j] = 0;
                continue;
            }

            int cap[2*MAXN][2*MAXN];
            buildNetwork(n, graph, i, j, cap);
            int src = i * 2 + 1;    // source_out
            int dst = j * 2;        // dest_in
            int total = 2 * n;
            result[i][j] = maxFlow(total, src, dst, cap);
        }
    }

    // print the all‑pairs vertex connectivity matrix
    std::cout << "All‑Pairs Vertex Connectivity (n = " << n << ")\n";
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            std::cout << result[i][j] << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
