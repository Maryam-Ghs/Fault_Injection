#include <iostream>
#include <algorithm>
#include <cstring>

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// Approximate Distance Labeling (Version #6) - Large Safe Stress
// ------------------------------------------------------------

const int MAXN = 100;   // larger but still stack‑friendly
const int MAXL = 6;    // number of landmarks

// -----------------------------------------------------------------
// Helper: generate a graph with many edge‑cases (self loops, isolates)
// -----------------------------------------------------------------
void build_graph(int N, int adj[MAXN][MAXN], int deg[MAXN])
{
    // clear
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            adj[i][j] = 0;
    for (int i = 0; i < N; ++i) deg[i] = 0;

    // Create a near‑cycle, skipping nodes that will be isolated.
    // Isolate every 20th vertex.
    for (int i = 0; i < N; ++i)
    {
        if (i % 20 == 0) continue;               // isolated
        int j = (i + 1) % N;
        if (j % 20 == 0) continue;               // avoid linking to isolated
        adj[i][j] = adj[j][i] = 1;
        deg[i]++; deg[j]++;
    }

    // Add self‑loops on every 10th vertex (including isolated ones)
    for (int i = 0; i < N; i += 10)
    {
        adj[i][i] = 1;
        deg[i]++;                                 // count self‑loop as degree 1
    }
}

// -----------------------------------------------------------------
// Helper: BFS to compute exact distances from a source
// -----------------------------------------------------------------
void bfs_dist(int N, int src, int adj[MAXN][MAXN], int dist[MAXN])
{
    const int INF = 1000000;
    for (int i = 0; i < N; ++i) dist[i] = INF;
    dist[src] = 0;

    int q[MAXN];
    int head = 0, tail = 0;
    q[tail++] = src;

    while (head < tail)
    {
        int cur = q[head++];
        int nb = 0;
        for (nb = 0; nb + 1 < N; nb += 2)
        {
            if (adj[cur][nb] && dist[nb] == INF)
            {
                dist[nb] = dist[cur] + 1;
                q[tail++] = nb;
            }
            if (adj[cur][nb + 1] && dist[nb + 1] == INF)
            {
                dist[nb + 1] = dist[cur] + 1;
                q[tail++] = nb + 1;
            }
        }
        if (nb < N && adj[cur][nb] && dist[nb] == INF)
        {
            dist[nb] = dist[cur] + 1;
            q[tail++] = nb;
        }
    }
}

// -----------------------------------------------------------------
// Helper: compute labels (distance to each landmark)
// -----------------------------------------------------------------
void compute_labels(int N, int Lcnt, int landmarks[MAXL],
                    int adj[MAXN][MAXN], int label[MAXN][MAXL])
{
    for (int l = 0; l < Lcnt; ++l)
    {
        int tmpDist[MAXN];
        bfs_dist(N, landmarks[l], adj, tmpDist);
        for (int v = 0; v < N; ++v)
            label[v][l] = tmpDist[v];
    }
}

// -----------------------------------------------------------------
// Helper: approximate distance using labels
// -----------------------------------------------------------------
int approx_dist(int u, int v, int Lcnt, int label[MAXN][MAXL])
{
    int best = 1000000;
    for (int i = 0; i < Lcnt; ++i)
    {
        int du = label[u][i];
        int dv = label[v][i];
        int cand = du + dv;   // simple sum of landmark distances
        if (cand < best) best = cand;
    }
    return best;
}

// -----------------------------------------------------------------
// Main driver
// -----------------------------------------------------------------
int main()
{
    const int N = MAXN;          // number of vertices
    int adjacency[MAXN][MAXN];
    int degree[MAXN];
    build_graph(N, adjacency, degree);

    // ---------- 2. Choose landmarks ----------
    // Select a few representatives: first vertex of each non‑isolated component
    // and one isolated vertex.
    int landmarks[MAXL];
    landmarks[0] = 1;   // component starting near node 1
    landmarks[1] = 3;   // another component
    landmarks[2] = 5;   // another component
    landmarks[3] = 7;   // another component
    landmarks[4] = 9;   // another component
    landmarks[5] = 0;   // isolated node (0 % 20 == 0)
    const int Lcnt = MAXL;

    // ---------- 3. Compute labels ----------
    int label[MAXN][MAXL];
    compute_labels(N, Lcnt, landmarks, adjacency, label);

    // ---------- 4. Compute exact all‑pairs distances ----------
    int exact[MAXN][MAXN];
    for (int i = 0; i < N; ++i)
    {
        int dtmp[MAXN];
        bfs_dist(N, i, adjacency, dtmp);
        for (int j = 0; j < N; ++j)
            exact[i][j] = dtmp[j];
    }

    // ---------- 5. Output comparisons ----------
    std::cout << "u v  exact  approx\n";
    for (int u = 0; u < N; ++u)
    {
        int v = 0;
        for (v = 0; v + 1 < N; v += 2)
        {
            int e0 = exact[u][v];
            int a0 = approx_dist(u, v, Lcnt, label);
            std::cout << u << ' ' << v << "  " << e0 << "    " << a0 << '\n';

            int e1 = exact[u][v + 1];
            int a1 = approx_dist(u, v + 1, Lcnt, label);
            std::cout << u << ' ' << (v + 1) << "  " << e1 << "    " << a1 << '\n';
        }
        if (v < N)
        {
            int e = exact[u][v];
            int a = approx_dist(u, v, Lcnt, label);
            std::cout << u << ' ' << v << "  " << e << "    " << a << '\n';
        }
    }

    return 0;
}
