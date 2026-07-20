#include <bits/stdc++.h>
using namespace std;
/* LLM input variant 8: sparse-skewed */
int main() {
    // ----- parameters -------------------------------------------------
    int vertexCount = 12;                     // size of the random graph
    int maxCap = 15;                          // maximal edge capacity

    // ----- generate a deterministic sparse‑skewed capacity matrix -----
    vector< vector<int> > capacityMatrix(vertexCount,
                                         vector<int>(vertexCount, 0));
    srand(42); // fixed seed for reproducibility
    for (int a = 0; a < vertexCount; ++a) {
        for (int b = a + 1; b < vertexCount; ++b) {
            int cap = 0;
            // Dense cluster among vertices 0‑3
            if (a < 4 && b < 4) {
                cap = rand() % (maxCap - 5 + 1) + 5; // capacities 5‑15
            }
            // Sparse, low‑capacity edges from cluster to the rest
            else if (a < 4 && b >= 4) {
                if ( (a + b) % 7 == 0 ) {
                    cap = 1; // occasional thin link
                }
            }
            // Remaining vertices (4‑11) stay mostly disconnected (cap = 0)
            capacityMatrix[a][b] = cap;
            capacityMatrix[b][a] = cap;       // undirected graph
        }
    }

    // ----- helper: BFS that records a path in the residual graph -------
    auto bfsPath = [&](int src, int dst,
                       const vector< vector<int> >& resid,
                       vector<int>& pred) -> bool {
        fill(pred.begin(), pred.end(), -1);
        queue<int> q;
        q.push(src);
        pred[src] = src;
        while (!q.empty()) {
            int cur = q.front(); q.pop();
            for (int nxt = 0; nxt < vertexCount; ++nxt) {
                if (pred[nxt] == -1 && resid[cur][nxt] > 0) {
                    pred[nxt] = cur;
                    if (nxt == dst) return true;
                    q.push(nxt);
                }
            }
        }
        return false;
    };

    // ----- Edmonds–Karp max‑flow (integral capacities) -----------------
    auto maxFlow = [&](int src, int dst,
                       const vector< vector<int> >& base) -> int {
        vector< vector<int> > resid = base;          // residual capacities
        vector<int> pred(vertexCount);
        int total = 0;

        while (bfsPath(src, dst, resid, pred)) {
            // find bottleneck
            int bottleneck = INT_MAX;
            for (int v = dst; v != src; v = pred[v]) {
                int u = pred[v];
                bottleneck = min(bottleneck, resid[u][v]);
            }
            // augment flow
            for (int v = dst; v != src; v = pred[v]) {
                int u = pred[v];
                resid[u][v] -= bottleneck;
                resid[v][u] += bottleneck;
            }
            total += bottleneck;
        }
        return total;
    };

    // ----- compute all‑pairs edge connectivity (min‑cut) ---------------
    vector< vector<int> > cutMatrix(vertexCount,
                                    vector<int>(vertexCount, 0));
    for (int i = 0; i < vertexCount; ++i) {
        for (int j = i + 1; j < vertexCount; ++j) {
            int flow = maxFlow(i, j, capacityMatrix);
            cutMatrix[i][j] = flow;
            cutMatrix[j][i] = flow;
        }
    }

    // ----- output ------------------------------------------------------
    cout << "All‑Pairs Edge Connectivity (minimum cut values)\n";
    for (int i = 0; i < vertexCount; ++i) {
        for (int j = 0; j < vertexCount; ++j) {
            cout << cutMatrix[i][j];
            if (j + 1 < vertexCount) cout << ' ';
        }
        cout << '\n';
    }
    return 0;
}
