/* LLM input variant 7: reverse-adversarial */
// Version 9 – All‑Pairs Shortest Path using Repeated Dijkstra
// -----------------------------------------------------------
// Generates a deterministic weighted directed graph on the stack,
// runs Dijkstra from every vertex, and prints the distance matrix.

#include <bits/stdc++.h>
using namespace std;

#define MAXV 8                     // maximum number of vertices
#define INF  1000000               // a value larger than any possible path

class Graph {
    int Vcnt;                     // number of vertices
    int adj[MAXV][MAXV];          // adjacency matrix (0 = no edge)

public:
    Graph(int n) : Vcnt(n) {
        // deterministic reverse‑ordered graph:
        // edge from higher‑index vertex to lower‑index vertex with weight 1,
        // no edge from lower to higher, diagonal zero.
        for (int i = 0; i < Vcnt; ++i) {
            for (int j = 0; j < Vcnt; ++j) {
                if (i == j) {
                    adj[i][j] = 0;
                } else if (i > j) {
                    adj[i][j] = 1;          // reverse edge, minimal weight
                } else {
                    adj[i][j] = 0;          // no forward edge
                }
            }
        }
    }

    // run Dijkstra from a given source and write distances into row 'src'
    void dijkstraFrom(int src, int row[MAXV]) {
        int dist[MAXV];
        bool done[MAXV];

        // initialise
        for (int i = 0; i < Vcnt; ++i) {
            dist[i] = INF;
            done[i] = false;
        }
        dist[src] = 0;

        // repeat Vcnt times (or until all reachable vertices are settled)
        int iter = 0;
        while (iter < Vcnt) {
            int u = -1;
            int best = INF;

            // pick the unsettled vertex with smallest distance
            for (int k = 0; k < Vcnt; ++k) {
                if (!done[k] && dist[k] < best) {
                    best = dist[k];
                    u = k;
                }
            }

            if (u == -1) break;          // remaining vertices are unreachable
            done[u] = true;

            // relax outgoing edges of u
            for (int v = 0; v < Vcnt; ++v) {
                if (!done[v] && adj[u][v] > 0) {
                    int nd = dist[u] + adj[u][v];
                    if (nd < dist[v]) dist[v] = nd;
                }
            }
            ++iter;
        }

        // copy result into the supplied row
        for (int i = 0; i < Vcnt; ++i) row[i] = dist[i];
    }

    // compute all‑pairs distances
    void allPairsShortest(int allDist[MAXV][MAXV]) {
        for (int s = 0; s < Vcnt; ++s) {
            dijkstraFrom(s, allDist[s]);
        }
    }

    // utility to print the adjacency matrix
    void printAdjacency() const {
        cout << "Adjacency matrix (weight 0 = no edge):\n";
        for (int i = 0; i < Vcnt; ++i) {
            for (int j = 0; j < Vcnt; ++j) {
                cout << adj[i][j] << ' ';
            }
            cout << '\n';
        }
        cout << '\n';
    }

    // utility to print a distance matrix
    static void printDist(const char* title, int mat[MAXV][MAXV], int n) {
        cout << title << ":\n";
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (mat[i][j] >= INF) cout << "INF ";
                else cout << mat[i][j] << ' ';
            }
            cout << '\n';
        }
        cout << '\n';
    }
};

int main() {
    // deterministic graph, no need for random seed
    Graph g(MAXV);

    // show the generated graph
    g.printAdjacency();

    // compute all‑pairs shortest paths
    int distMat[MAXV][MAXV];
    g.allPairsShortest(distMat);

    // display results
    Graph::printDist("All‑Pairs Shortest Path distances", distMat, MAXV);

    return 0;
}
