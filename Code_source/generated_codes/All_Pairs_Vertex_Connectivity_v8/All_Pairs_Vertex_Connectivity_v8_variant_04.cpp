#include <cstdio>
#include <vector>

/* LLM input variant 4: signed-extremes */

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
            if (i == s || i == t) {
                cap[in][out] = big;   // source / sink are unlimited
            } else {
                // mix of positive, zero, and negative capacities
                if (i % 3 == 0) cap[in][out] = 1;      // positive
                else if (i % 3 == 1) cap[in][out] = 0; // zero
                else cap[in][out] = -1;               // negative (unused)
            }
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
    /* ---- generate a graph rich in edge‑cases ---- */
    int N = 9;                           // 7‑vertex component + 2 isolated vertices
    VertexConnect g(N);

    /* a line 0‑1‑2‑3‑4‑5‑6 */
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 5);
    g.addEdge(5, 6);

    /* extra chords and a triangle within the component */
    g.addEdge(0, 2);
    g.addEdge(2, 4);
    g.addEdge(0, 5);
    g.addEdge(5, 3);
    g.addEdge(3, 0);
    /* vertices 7 and 8 stay isolated – pure edge‑cases */

    std::vector<std::vector<int>> ans = g.allPairs();

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%d ", ans[i][j]);
        }
        printf("\n");
    }
    return 0;
}
