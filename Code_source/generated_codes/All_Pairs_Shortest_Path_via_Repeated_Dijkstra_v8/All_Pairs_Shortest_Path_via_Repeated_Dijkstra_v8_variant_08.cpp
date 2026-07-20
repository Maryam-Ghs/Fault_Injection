#include <iostream>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

int main() {
    // version #8
    const int V = 10;
    const int INF = 1000000;

    // predefined sparse adjacency matrix (weight = INF means no edge)
    int edgeW[10][10] = {
        {0,     5,   INF, INF, 100, INF, INF, INF, INF, 500},
        {5,     0,     7, INF, INF, 200, INF, INF, INF, INF},
        {INF,   7,     0,   3, INF, INF, INF, INF, INF, INF},
        {INF, INF,     3,   0, INF, INF, INF, INF, INF, INF},
        {100, INF, INF, INF,   0, INF,   2, INF, INF, INF},
        {INF, 200, INF, INF, INF,   0, INF,   1, INF, INF},
        {INF, INF, INF, INF,   2, INF,   0, INF, INF, INF},
        {INF, INF, INF, INF, INF,   1, INF,   0, INF, INF},
        {INF, INF, INF, INF, INF, INF, INF, INF,   0, INF},
        {500, INF, INF, INF, INF, INF, INF, INF, INF,   0}
    };

    // container for all‑pairs shortest distances
    int allDist[10][10];

    // Dijkstra as a lambda (modular function, still inside main)
    auto runDijkstra = [&](int src, int* result) {
        int dist[10];
        int used[10];
        int i = 0;
        while (i < V) {
            dist[i] = INF;
            used[i] = 0;
            i = i + 1;
        }
        dist[src] = 0;

        while (true) {
            int u = -1;
            int best = INF;
            int j = 0;
            while (j < V) {
                int cand = dist[j];
                int free = (used[j] == 0);
                if (free && cand < best) {
                    best = cand;
                    u = j;
                }
                j = j + 1;
            }
            if (u == -1) break;
            used[u] = 1;

            int v = 0;
            while (v < V) {
                int w = edgeW[u][v];
                int reachable = (w < INF);
                int notVis = (used[v] == 0);
                if (reachable && notVis) {
                    int alt = dist[u] + w;
                    if (alt < dist[v]) {
                        dist[v] = alt;
                    }
                }
                v = v + 1;
            }
        }

        int k = 0;
        while (k < V) {
            result[k] = dist[k];
            k = k + 1;
        }
    };

    // run Dijkstra from every vertex
    int s = 0;
    while (s < V) {
        runDijkstra(s, allDist[s]);
        s = s + 1;
    }

    // print the all‑pairs distance matrix
    int r = 0;
    while (r < V) {
        int c = 0;
        while (c < V) {
            int val = allDist[r][c];
            if (val >= INF) {
                std::cout << "INF";
            } else {
                std::cout << std::setw(3) << val;
            }
            if (c < V - 1) std::cout << " ";
            c = c + 1;
        }
        std::cout << "\n";
        r = r + 1;
    }

    return 0;
}
