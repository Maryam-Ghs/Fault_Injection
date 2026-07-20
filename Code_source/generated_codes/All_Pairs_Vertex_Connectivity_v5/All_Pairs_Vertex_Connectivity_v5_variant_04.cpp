#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>

struct Edge {
    int to;
    int rev;
    int cap;
};

void addEdge(std::vector<std::vector<Edge>>& g, int a, int b, int c) {
    Edge f{b, (int)g[b].size(), c};
    Edge r{a, (int)g[a].size(), 0};
    g[a].push_back(f);
    g[b].push_back(r);
}

/* LLM input variant 4: signed-extremes */

/* Dinic helper: level graph */
bool bfsLevel(std::vector<std::vector<Edge>>& g, int src, int snk, std::vector<int>& lev) {
    std::fill(lev.begin(), lev.end(), -1);
    std::queue<int> q;
    lev[src] = 0;
    q.push(src);
    while (!q.empty()) {
        int cur = q.front(); q.pop();
        for (size_t i = 0; i < g[cur].size(); ++i) {
            Edge &e = g[cur][i];
            if (e.cap > 0 && lev[e.to] < 0) {
                lev[e.to] = lev[cur] + 1;
                q.push(e.to);
            }
        }
    }
    return lev[snk] >= 0;
}

/* Dinic helper: blocking flow */
int dfsBlock(std::vector<std::vector<Edge>>& g, int cur, int snk, int flow,
             std::vector<int>& lev, std::vector<int>& it) {
    if (cur == snk) return flow;
    for (int &i = it[cur]; i < (int)g[cur].size(); ++i) {
        Edge &e = g[cur][i];
        if (e.cap > 0 && lev[e.to] == lev[cur] + 1) {
            int pushed = dfsBlock(g, e.to, snk,
                                  flow < e.cap ? flow : e.cap,
                                  lev, it);
            if (pushed > 0) {
                e.cap -= pushed;
                g[e.to][e.rev].cap += pushed;
                return pushed;
            }
        }
    }
    return 0;
}

/* Max‑flow using Dinic */
int maxFlow(std::vector<std::vector<Edge>>& g, int src, int snk) {
    int total = 0;
    std::vector<int> lev(g.size());
    while (bfsLevel(g, src, snk, lev)) {
        std::vector<int> it(g.size(), 0);
        while (true) {
            int f = dfsBlock(g, src, snk, 1e9, lev, it);
            if (f == 0) break;
            total += f;
        }
    }
    return total;
}

/* Build vertex‑split network for a given pair (s,t) */
int vertexCut(const std::vector<std::vector<int>>& adj, int s, int t) {
    int n = adj.size();
    int sz = 2 * n;                    // each vertex -> in/out
    std::vector<std::vector<Edge>> net(sz);
    // split vertices with mixed signed capacities
    for (int v = 0; v < n; ++v) {
        int in = v;
        int out = v + n;
        int cap;
        if (v == s || v == t) {
            cap = 1e6;                 // source/target infinite
        } else if (v % 3 == 0) {
            cap = -5;                  // negative capacity (ignored by flow)
        } else if (v % 3 == 1) {
            cap = 0;                   // zero capacity
        } else {
            cap = 2;                   // small positive capacity
        }
        addEdge(net, in, out, cap);
    }
    // original edges become out->in arcs
    for (int u = 0; u < n; ++u) {
        for (size_t k = 0; k < adj[u].size(); ++k) {
            int v = adj[u][k];
            int uout = u + n;
            int vin = v;
            addEdge(net, uout, vin, 1e6);
        }
    }
    int src = s + n;   // out part of s
    int snk = t;       // in part of t
    return maxFlow(net, src, snk);
}

/* Compute all‑pairs vertex connectivity */
std::vector<std::vector<int>> allPairsConn(const std::vector<std::vector<int>>& adj) {
    int n = adj.size();
    std::vector<std::vector<int>> ans(n, std::vector<int>(n, 0));
    for (int a = 0; a < n; ++a) {
        for (int b = a + 1; b < n; ++b) {
            int cut = vertexCut(adj, a, b);
            ans[a][b] = ans[b][a] = cut;
        }
    }
    return ans;
}

/* Deterministic medium‑density graph */
std::vector<std::vector<int>> genGraph(int vcnt) {
    std::vector<std::vector<int>> mat(vcnt);
    // Fixed edges for 6 vertices (if vcnt >= 6)
    if (vcnt >= 6) {
        mat[0] = {1,2};
        mat[1] = {0,3};
        mat[2] = {0,3,4};
        mat[3] = {1,2,5};
        mat[4] = {2};
        mat[5] = {3};
        // Ensure symmetry
        for (int i = 0; i < vcnt; ++i) {
            for (int j : mat[i]) {
                if (std::find(mat[j].begin(), mat[j].end(), i) == mat[j].end())
                    mat[j].push_back(i);
            }
        }
    }
    return mat;
}

/* Print matrix */
void show(const std::vector<std::vector<int>>& m) {
    int n = m.size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            std::cout << m[i][j];
            if (j + 1 < n) std::cout << ' ';
        }
        std::cout << '\n';
    }
}

/* Entry point */
int main() {
    int verts = 6;                       // deterministic size
    auto graph = genGraph(verts);
    auto conn = allPairsConn(graph);
    std::cout << "All‑Pairs Vertex Connectivity:\n";
    show(conn);
    return 0;
}
