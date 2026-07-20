/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <queue>
#include <algorithm>
#include <cstring>

// ------------------------------------------------------------
// Approximate Distance Labeling (Version #6)
// ------------------------------------------------------------

// -----------------------------------------------------------------
// Helper: generate a sparse, skewed graph (high-degree hub, many isolates)
// -----------------------------------------------------------------
void build_graph(int N, int adj[12][12], int deg[12])
{
    // clear
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            adj[i][j] = 0;
    for (int i = 0; i < N; ++i) deg[i] = 0;

    // Hub node 0 connects to many others (skewed degree)
    adj[0][1] = adj[1][0] = 1; deg[0]++; deg[1]++;
    adj[0][2] = adj[2][0] = 1; deg[0]++; deg[2]++;
    adj[0][3] = adj[3][0] = 1; deg[0]++; deg[3]++;
    adj[0][4] = adj[4][0] = 1; deg[0]++; deg[4]++;
    adj[0][5] = adj[5][0] = 1; deg[0]++; deg[5]++;
    adj[0][6] = adj[6][0] = 1; deg[0]++; deg[6]++;

    // Additional small edges to create a tiny component
    adj[1][2] = adj[2][1] = 1; deg[1]++; deg[2]++;

    // Small isolated component 7–8
    adj[7][8] = adj[8][7] = 1; deg[7]++; deg[8]++;

    // Self‑loop on node 9
    adj[9][9] = 1; deg[9]++;

    // Nodes 10 and 11 stay isolated (deg = 0)
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
    // ---------- 1. Build sparse, skewed graph ----------
    const int N = 12;                     // number of vertices (stack bound)
    int adjacency[12][12];
    int degree[12];
    build_graph(N, adjacency, degree);

    // ---------- 2. Choose landmarks (one per component, plus isolates) ----------
    int landmarks[4];
    landmarks[0] = 0;   // hub component
    landmarks[1] = 7;   // small component 7–8
    landmarks[2] = 9;   // self‑loop node
    landmarks[3] = 10;  // isolated node
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
