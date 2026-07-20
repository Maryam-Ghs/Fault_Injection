/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <queue>
#include <algorithm>
#include <cstring>

// ------------------------------------------------------------
// Approximate Distance Labeling (Version #6) - Minimal Variant
// ------------------------------------------------------------

const int MAXN = 2;   // minimal nontrivial number of vertices
const int MAXL = 1;   // minimal number of landmarks

// -----------------------------------------------------------------
// Helper: generate a minimal graph (edge + self‑loop)
// -----------------------------------------------------------------
void build_graph(int N, int adj[MAXN][MAXN], int deg[MAXN])
{
    // clear
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            adj[i][j] = 0;
    for (int i = 0; i < N; ++i) deg[i] = 0;

    // edge 0–1 and self‑loop on 0
    adj[0][1] = adj[1][0] = 1; deg[0]++; deg[1]++;
    adj[0][0] = 1; deg[0]++;
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
            if (adj[cur][nb])
            {
                if (dist[nb] == INF)
                {
                    dist[nb] = dist[cur] + 1;
                    q[tail++] = nb;
                }
            }
            if (adj[cur][nb + 1])
            {
                if (dist[nb + 1] == INF)
                {
                    dist[nb + 1] = dist[cur] + 1;
                    q[tail++] = nb + 1;
                }
            }
        }
        if (nb < N)
        {
            if (adj[cur][nb])
            {
                if (dist[nb] == INF)
                {
                    dist[nb] = dist[cur] + 1;
                    q[tail++] = nb;
                }
            }
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
        int sum = du + dv;
        int cand = sum;
        if (cand < best) best = cand;
    }
    return best;
}

// -----------------------------------------------------------------
// Main driver
// -----------------------------------------------------------------
int main()
{
    // ---------- 1. Build minimal graph ----------
    const int N = MAXN;
    int adjacency[MAXN][MAXN];
    int degree[MAXN];
    build_graph(N, adjacency, degree);

    // ---------- 2. Choose minimal landmark ----------
    int landmarks[MAXL];
    landmarks[0] = 0;   // use node 0 as the sole landmark
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
