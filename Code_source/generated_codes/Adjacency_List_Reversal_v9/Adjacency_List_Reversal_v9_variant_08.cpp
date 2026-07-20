#include <iostream>
#include <cstdlib>
#include <ctime>

#define MAXV 8          // maximum vertices
#define MAXE 20         // maximum total edges
#define MAXD 5          // maximum out-degree per vertex

/* LLM input variant 8: sparse-skewed */

// ------------------------------------------------------------
// Generate a deterministic skewed directed graph using stack‑allocated arrays
// ------------------------------------------------------------
void genGraph(int vcnt, int deg[], int adj[][MAXD]) {
    // initialise degrees
    for (int i = 0; i < vcnt; ++i) deg[i] = 0;

    // Skewed pattern:
    // Vertex 0 connects to as many other vertices as allowed (up to MAXD)
    for (int dst = 1; dst < vcnt && deg[0] < MAXD; ++dst) {
        adj[0][deg[0]++] = dst;
    }
    // All other vertices connect only to vertex 0 (if degree permits)
    for (int src = 1; src < vcnt; ++src) {
        if (deg[src] < MAXD) {
            adj[src][deg[src]++] = 0;
        }
    }
}

// ------------------------------------------------------------
// Build the transpose (reversed adjacency list)
// ------------------------------------------------------------
void revGraph(int vcnt, int deg[], int adj[][MAXD],
              int rdeg[], int radj[][MAXD]) {
    // initialise reverse degrees
    for (int i = 0; i < vcnt; ++i) rdeg[i] = 0;

    // iterate over original edges and insert reversed ones
    for (int u = 0; u < vcnt; ++u) {
        for (int i = 0; i < deg[u]; ++i) {
            int v = adj[u][i];
            // fused: increment reverse degree and assign in one statement
            radj[v][rdeg[v]++] = u;
        }
    }
}

// ------------------------------------------------------------
// Print adjacency list of a graph
// ------------------------------------------------------------
void printGraph(const char* title, int vcnt, int deg[], int adj[][MAXD]) {
    std::cout << title << ":\n";
    for (int i = 0; i < vcnt; ++i) {
        std::cout << "  " << i << ": ";
        for (int j = 0; j < deg[i]; ++j)
            std::cout << adj[i][j] << (j + 1 == deg[i] ? "" : ", ");
        std::cout << '\n';
    }
    std::cout << '\n';
}

// ------------------------------------------------------------
// Entry point – version #9
// ------------------------------------------------------------
int main() {
    std::srand(12345);                     // deterministic pseudo‑random

    int vertices = 6;                      // medium‑sized graph
    int outDeg[MAXV];                      // out‑degree per vertex
    int outAdj[MAXV][MAXD];                // adjacency list (stack)

    genGraph(vertices, outDeg, outAdj);    // create deterministic skewed graph
    printGraph("Original graph", vertices, outDeg, outAdj);

    int revDeg[MAXV];                      // reverse out‑degree
    int revAdj[MAXV][MAXD];                // reversed adjacency list

    revGraph(vertices, outDeg, outAdj, revDeg, revAdj);
    printGraph("Reversed graph", vertices, revDeg, revAdj);

    return 0;
}
