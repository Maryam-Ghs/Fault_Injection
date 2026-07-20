/* LLM input variant 2: small-diverse */
#include <bits/stdc++.h>
using namespace std;

#define MAXV 8                     // maximum number of vertices
#define INF  1000000               // a value larger than any possible path

class Graph {
    int Vcnt;                     // number of vertices
    int adj[MAXV][MAXV];          // adjacency matrix (0 = no edge)

public:
    Graph(int n) : Vcnt(n) {
        // initialise all entries to 0 (no edge)
        for (int i = 0; i < MAXV; ++i)
            for (int j = 0; j < MAXV; ++j)
                adj[i][j] = 0;

        // deterministic small‑diverse directed graph for Vcnt = 6
        if (Vcnt >= 6) {
            adj[0][1] = 2;  // edge 0 -> 1 weight 2
            adj[0][3] = 1;  // edge 0 -> 3 weight 1

            adj[1][0] = 3;  // edge 1 -> 0 weight 3
            adj[1][2] = 4;  // edge 1 -> 2 weight 4

            adj[2][1] = 5;  // edge 2 -> 1 weight 5
            adj[2][4] = 2;  // edge 2 -> 4 weight 2

            adj[3][2] = 6;  // edge 3 -> 2 weight 6
            adj[3][5] = 1;  // edge 3 -> 5 weight 1

            adj[4][3] = 7;  // edge 4 -> 3 weight 7
            adj[4][5] = 3;  // edge 4 -> 5 weight 3

            adj[5][0] = 8;  // edge 5 -> 0 weight 8
            adj[5][4] = 4;  // edge 5 -> 4 weight 4
        }

        // ensure diagonal is zero
        for (int i = 0; i < Vcnt; ++i) adj[i][i] = 0;
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
    // deterministic graph with 6 vertices
    Graph g(6);

    // show the generated graph
    g.printAdjacency();

    // compute all‑pairs shortest paths
    int distMat[MAXV][MAXV];
    g.allPairsShortest(distMat);

    // display results
    Graph::printDist("All‑Pairs Shortest Path distances", distMat, 6);

    return 0;
}
