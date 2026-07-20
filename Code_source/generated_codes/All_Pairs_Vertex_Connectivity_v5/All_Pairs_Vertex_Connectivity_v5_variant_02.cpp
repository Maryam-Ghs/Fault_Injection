#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>

/* LLM input variant 2: small-diverse */

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
    int sz = 2 * n;
    std::vector<std::vector<Edge>> net(sz);
    for (int v = 0; v < n; ++v) {
        int in = v;
        int out = v + n;
        int cap = (v == s || v == t) ? 1e6 : 1;
        addEdge(net, in, out, cap);
    }
    for (int u = 0; u < n; ++u) {
        for (size_t k = 0; k < adj[u].size(); ++k) {
            int v = adj[u][k];
            int uout = u + n;
            int vin = v;
            addEdge(net, uout, vin, 1e6);
        }
    }
    int src = s + n;
    int snk = t;
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

/* Generate a deterministic small diverse undirected graph */
std::vector<std::vector<int>> genGraph(int vcnt) {
    std::vector<std::vector<int>> mat(vcnt);
    // Edge list chosen to give varied degrees and connectivity
    const std::pair<int,int> edges[] = {
        {0,1},{0,2},{1,3},{2,3},{3,4},
        {4,5},{5,6},{2,6},{1,5},{0,6}
    };
    for (auto &e : edges) {
        int a = e.first, b = e.second;
        if (a < vcnt && b < vcnt) {
            mat[a].push_back(b);
            mat[b].push_back(a);
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
    std::srand((unsigned)std::time(0));
    int verts = 7;
    auto graph = genGraph(verts);
    auto conn = allPairsConn(graph);
    std::cout << "All‑Pairs Vertex Connectivity:\n";
    show(conn);
    return 0;
}
