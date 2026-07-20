#include <cstdio>
#include <vector>

/* LLM input variant 10: large-safe-stress */

class VertexConnect {
public:
    int vCount;
    std::vector<std::vector<int>> links;

    VertexConnect(int sz) : vCount(sz), links(sz) {}

    void addEdge(int a, int b) {
        links[a].push_back(b);
        links[b].push_back(a);
    }

    std::vector<std::vector<int>> allPairs() {
        std::vector<std::vector<int>> mat(vCount, std::vector<int>(vCount, 0));
        for (int src = 0; src < vCount; ++src) {
            for (int dst = src + 1; dst < vCount; ++dst) {
                int cut = vertexCut(src, dst);
                mat[src][dst] = cut;
                mat[dst][src] = cut;
            }
        }
        return mat;
    }

private:
    int big = 1000000;          // a value larger than any possible cut

    int vertexCut(int s, int t) {
        int tot = 2 * vCount;                     // split each vertex
        std::vector<std::vector<int>> cap(tot,
                                          std::vector<int>(tot, 0));

        /* split vertices: in = i, out = i+vCount */
        for (int i = 0; i < vCount; ++i) {
            int in = i;
            int out = i + vCount;
            if (i == s || i == t) cap[in][out] = big;   // source / sink are unlimited
            else                 cap[in][out] = 1;      // ordinary vertices cost 1
        }

        /* original edges become infinite‑capacity arcs between outs and ins */
        for (int u = 0; u < vCount; ++u) {
            for (int v : links[u]) {
                int uOut = u + vCount;
                int vIn  = v;
                cap[uOut][vIn] = big;
            }
        }

        int source = s + vCount;   // start from s_out
        int sink   = t;            // end at t_in
        int flow = 0;

        while (true) {
            std::vector<int> parent(tot, -1);
            std::vector<int> queue;
            queue.reserve(tot);
            int qh = 0;
            queue.push_back(source);
            parent[source] = source;

            /* BFS to find augmenting path */
            while (qh < (int)queue.size() && parent[sink] == -1) {
                int cur = queue[qh++];
                for (int nxt = 0; nxt < tot; ++nxt) {
                    if (parent[nxt] == -1 && cap[cur][nxt] > 0) {
                        parent[nxt] = cur;
                        queue.push_back(nxt);
                    }
                }
            }

            if (parent[sink] == -1) break;   // no augmenting path

            int inc = big;
            for (int v = sink; v != source; v = parent[v]) {
                int u = parent[v];
                if (cap[u][v] < inc) inc = cap[u][v];
            }

            for (int v = sink; v != source; v = parent[v]) {
                int u = parent[v];
                cap[u][v] -= inc;
                cap[v][u] += inc;
            }

            flow += inc;
            if (flow >= big) break;          // enough – treat as infinite
        }
        return flow;
    }
};

int main() {
    /* ---- generate a relatively large, safe stress test graph ---- */
    const int N = 100;                     // 99‑vertex dense component + 1 isolated vertex
    VertexConnect g(N);

    // Fully connect vertices 0 … 98 (complete subgraph)
    for (int i = 0; i < N - 1; ++i) {
        for (int j = i + 1; j < N - 1; ++j) {
            g.addEdge(i, j);
        }
    }
    // Vertex N‑1 (index 99) stays isolated – edge case

    std::vector<std::vector<int>> ans = g.allPairs();

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%d ", ans[i][j]);
        }
        printf("\n");
    }
    return 0;
}
