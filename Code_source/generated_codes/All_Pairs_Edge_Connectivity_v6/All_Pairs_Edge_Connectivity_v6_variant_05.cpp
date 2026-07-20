#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 5: duplicate-heavy */

int main() {
    // Version #6 – All‑Pairs Edge Connectivity (manual unrolling, all in main)

    // ----- generate a duplicate‑heavy deterministic undirected graph -----
    int seed = 12345;                 // deterministic seed (kept for consistency)
    srand(seed);
    int vertCnt = 7;                  // number of vertices (medium size)
    int edgeProb = 100;               // set to 100% to create all possible edges

    // adjacency matrix of capacities (undirected → two directed edges)
    vector< vector<int> > cap(vertCnt, vector<int>(vertCnt, 0));

    for (int i = 0; i < vertCnt; ++i) {
        for (int j = i + 1; j < vertCnt; ++j) {
            // deterministic duplicate‑heavy pattern: every possible edge has the same capacity
            cap[i][j] = 3;            // uniform capacity creates many ties
            cap[j][i] = 3;
        }
    }

    // ----- helper lambda: Edmonds‑Karp max‑flow for a given source‑target -----
    auto maxFlow = [&](int src, int dst) -> int {
        // residual capacities copy
        vector< vector<int> > res = cap;
        int flow = 0;
        vector<int> parent(vertCnt);
        vector<int> q(vertCnt);
        while (true) {
            // ---- BFS to find augmenting path (manual unrolling of neighbor loop) ----
            fill(parent.begin(), parent.end(), -1);
            int head = 0, tail = 0;
            q[tail++] = src;
            parent[src] = src;
            while (head < tail && parent[dst] == -1) {
                int cur = q[head++];
                // unroll neighbours in groups of four
                int nb = 0;
                for (; nb + 3 < vertCnt; nb += 4) {
                    if (parent[nb] == -1 && res[cur][nb] > 0) {
                        parent[nb] = cur; q[tail++] = nb;
                    }
                    if (parent[nb + 1] == -1 && res[cur][nb + 1] > 0) {
                        parent[nb + 1] = cur; q[tail++] = nb + 1;
                    }
                    if (parent[nb + 2] == -1 && res[cur][nb + 2] > 0) {
                        parent[nb + 2] = cur; q[tail++] = nb + 2;
                    }
                    if (parent[nb + 3] == -1 && res[cur][nb + 3] > 0) {
                        parent[nb + 3] = cur; q[tail++] = nb + 3;
                    }
                }
                for (; nb < vertCnt; ++nb) {
                    if (parent[nb] == -1 && res[cur][nb] > 0) {
                        parent[nb] = cur; q[tail++] = nb;
                    }
                }
            }
            if (parent[dst] == -1) break;          // no augmenting path

            // ---- find bottleneck (could be >1 due to uniform capacity) ----
            int inc = INT_MAX;
            for (int v = dst; v != src; v = parent[v]) {
                int u = parent[v];
                if (res[u][v] < inc) inc = res[u][v];
            }

            // ---- augment flow and update residual graph ----
            for (int v = dst; v != src; v = parent[v]) {
                int u = parent[v];
                res[u][v] -= inc;
                res[v][u] += inc;
            }
            flow += inc;
        }
        return flow;    // equals edge‑connectivity for unit‑capacity graph (here uniform capacity)
    };

    // ----- compute all‑pairs edge connectivity matrix -----
    vector< vector<int> > conn(vertCnt, vector<int>(vertCnt, 0));
    for (int a = 0; a < vertCnt; ++a) {
        for (int b = a + 1; b < vertCnt; ++b) {
            int val = maxFlow(a, b);
            conn[a][b] = val;
            conn[b][a] = val;
        }
    }

    // ----- print the generated graph (adjacency list) -----
    cout << "Generated undirected graph (capacity = 3 edges):\n";
    for (int i = 0; i < vertCnt; ++i) {
        cout << i << ": ";
        for (int j = 0; j < vertCnt; ++j) {
            if (cap[i][j] > 0) cout << j << " ";
        }
        cout << "\n";
    }
    cout << "\nAll‑Pairs Edge Connectivity matrix:\n";
    for (int i = 0; i < vertCnt; ++i) {
        for (int j = 0; j < vertCnt; ++j) {
            cout << conn[i][j] << " ";
        }
        cout << "\n";
    }
    return 0;
}
