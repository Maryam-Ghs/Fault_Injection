/* LLM input variant 4: signed-extremes */
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

    // Edge‑case heavy manual edge list with mixed signed values
    // 0–1 (negative weight), 0–2 (positive), 1–1 (self‑loop negative), 2 isolated,
    // 3–4 (positive), 3–5 (positive), 4–5 (positive), 5–5 (self‑loop positive)
    // 6–7 (positive), 6–8 (negative), 7–8 (positive), 8–9 (positive), 9–9 (self‑loop negative)
    // 10 isolated, 11–0 (wrap‑around positive)
    // Adding parallel edge 0–1 again (ignored)
    adj[0][1] = adj[1][0] = -1;  deg[0]--; deg[1]--;
    adj[0][2] = adj[2][0] = 1;   deg[0]++; deg[2]++;
    adj[0][1] = adj[1][0] = -1;  // parallel, no extra degree change
    adj[1][1] = -1;               deg[1]--; // self‑loop negative

    adj[3][4] = adj[4][3] = 1;    deg[3]++; deg[4]++;
    adj[3][5] = adj[5][3] = 1;    deg[3]++; deg[5]++;
    adj[4][5] = adj[5][4] = 1;    deg[4]++; deg[5]++;
    adj[5][5] = 1;                deg[5]++; // self‑loop positive

    adj[6][7] = adj[7][6] = 1;    deg[6]++; deg[7]++;
    adj[6][8] = adj[8][6] = -1;   deg[6]--; deg[8]--;
    adj[7][8] = adj[8][7] = 1;    deg[7]++; deg[8]++;
    adj[8][9] = adj[9][8] = 1;    deg[8]++; deg[9]++;
    adj[9][9] = -1;               deg[9]--; // self‑loop negative

    // node 10 stays isolated (deg[10] = 0)

    adj[11][0] = adj[0][11] = 1; deg[11]++; deg[0]++;

    // Inject explicit signed extremes into degree array
    deg[0] = -5;   // negative degree (unused later)
    deg[2] = 0;    // zero degree (already isolated)
    deg[4] = 7;    // positive degree beyond actual edge count (still harmless)
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
    // pick first node of each connected component, plus an isolated node
    int landmarks[4];
    landmarks[0] = 0;   // component containing node 0
    landmarks[1] = 3;   // component containing node 3
    landmarks[2] = 6;   // component containing node 6
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
