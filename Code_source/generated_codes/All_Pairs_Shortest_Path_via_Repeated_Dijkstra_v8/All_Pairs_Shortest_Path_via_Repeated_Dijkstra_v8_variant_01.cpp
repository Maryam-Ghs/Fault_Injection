#include <iostream>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */

int main() {
    // version #8
    const int V = 2;
    const int INF = 1000000;

    // predefined adjacency matrix (weight = INF means no edge)
    int edgeW[2][2] = {
        {0,   5},
        {INF, 0}
    };

    // container for all‑pairs shortest distances
    int allDist[2][2];

    // Dijkstra as a lambda (modular function, still inside main)
    auto runDijkstra = [&](int src, int* result) {
        int dist[2];
        int used[2];
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
