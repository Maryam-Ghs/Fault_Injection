/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <queue>
#include <algorithm>
#include <cstring>

// ------------------------------------------------------------
// Approximate Distance Labeling (Version #6)
// ------------------------------------------------------------

// -----------------------------------------------------------------
// Helper: generate a graph with many edge‑cases (self loops, isolates)
// -----------------------------------------------------------------
void build_graph(int N, int adj[12][12], int deg[12])
{
    // clear
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            adj[i][j] = 0;
    for (int i = 0; i < N; ++i) deg[i] = 0;

    // Edge‑case heavy reversed edges
    // Reverse chain: 11‑10‑9‑...‑0, with parallel edge and self‑loops,
    // plus a couple of cross edges to increase complexity.
    adj[11][10] = adj[10][11] = 1; deg[11]++; deg[10]++;
    // parallel duplicate (no extra degree)
    adj[11][10] = adj[10][11] = 1;

    adj[10][9] = adj[9][10] = 1; deg[10]++; deg[9]++;
    adj[9][8] = adj[8][9] = 1; deg[9]++; deg[8]++;
    adj[8][7] = adj[7][8] = 1; deg[8]++; deg[7]++;
    adj[7][6] = adj[6][7] = 1; deg[7]++; deg[6]++;
    adj[6][5] = adj[5][6] = 1; deg[6]++; deg[5]++;
    adj[5][4] = adj[4][5] = 1; deg[5]++; deg[4]++;
    adj[4][3] = adj[3][4] = 1; deg[4]++; deg[3]++;
    adj[3][2] = adj[2][3] = 1; deg[3]++; deg[2]++;
    adj[2][1] = adj[1][2] = 1; deg[2]++; deg[1]++;
    adj[1][0] = adj[0][1] = 1; deg[1]++; deg[0]++;

    // self‑loops
    adj[11][11] = 1; deg[11]++;
    adj[0][0]   = 1; deg[0]++;

    // additional cross edges (non‑sequential)
    adj[8][2] = adj[2][8] = 1; deg[8]++; deg[2]++;
    adj[9][3] = adj[3][9] = 1; deg[9]++; deg[3]++;
}

// -----------------------------------------------------------------
// Helper: BFS to compute exact distances from a source
// -----------------------------------------------------------------
void bfs_dist(int N, int src, int adj[12][12], int dist[12])
{
    const int INF = 1000000;
    for (int i = 0; i < N; ++i) dist[i] = INF;
    dist[src] = 0;

    // static queue on stack
    int q[12];
    int head = 0, tail = 0;
    q[tail++] = src;

    while (head < tail)
    {
        int cur = q[head++];
        // manual unroll of neighbor scan (step = 2)
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
        // tail element if N is odd
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
void compute_labels(int N, int Lcnt, int landmarks[4],
                    int adj[12][12], int label[12][4])
{
    // for each landmark, run BFS and store distances
    for (int l = 0; l < Lcnt; ++l)
    {
        int tmpDist[12];
        bfs_dist(N, landmarks[l], adj, tmpDist);
        // copy to label matrix
        for (int v = 0; v < N; ++v)
            label[v][l] = tmpDist[v];
    }
}

// -----------------------------------------------------------------
// Helper: approximate distance using labels
// -----------------------------------------------------------------
int approx_dist(int u, int v, int Lcnt, int label[12][4])
{
    // expanded multi‑step min computation
    int best = 1000000;
    for (int i = 0; i < Lcnt; ++i)
    {
        int du = label[u][i];
        int dv = label[v][i];
        int sum = du + dv;          // step 1
        int cand = sum;             // step 2 (could add extra transform)
        if (cand < best) best = cand;
    }
    return best;
}

// -----------------------------------------------------------------
// Main driver
// -----------------------------------------------------------------
int main()
{
    // ---------- 1. Build edge‑case heavy graph ----------
    const int N = 12;                     // number of vertices (stack bound)
    int adjacency[12][12];
    int degree[12];
    build_graph(N, adjacency, degree);

    // ---------- 2. Choose landmarks (edge‑case aware) ----------
    // pick vertices from the reversed chain, including high‑index nodes
    int landmarks[4];
    landmarks[0] = 11;   // high‑index end of chain
    landmarks[1] = 8;    // middle‑high
    landmarks[2] = 4;    // middle‑low
    landmarks[3] = 0;    // low‑index with self‑loop
    const int Lcnt = 4;

    // ---------- 3. Compute labels ----------
    int label[12][4];
    compute_labels(N, Lcnt, landmarks, adjacency, label);

    // ---------- 4. Compute exact all‑pairs distances ----------
    int exact[12][12];
    for (int i = 0; i < N; ++i)
    {
        int dtmp[12];
        bfs_dist(N, i, adjacency, dtmp);
        // copy row
        for (int j = 0; j < N; ++j)
            exact[i][j] = dtmp[j];
    }

    // ---------- 5. Output comparisons ----------
    std::cout << "u v  exact  approx\n";
    for (int u = 0; u < N; ++u)
    {
        // manual unroll of inner loop (step = 2)
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
