#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 1: minimal-boundary */

int main() {
    // ----- parameters -------------------------------------------------
    int vertexCount = 2;                     // minimal nontrivial graph size
    int maxCap = 1;                          // minimal positive capacity

    // ----- generate a deterministic undirected capacity matrix ----------
    vector< vector<int> > capacityMatrix(vertexCount,
                                         vector<int>(vertexCount, 0));
    // single edge with capacity 1
    capacityMatrix[0][1] = 1;
    capacityMatrix[1][0] = 1;

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
