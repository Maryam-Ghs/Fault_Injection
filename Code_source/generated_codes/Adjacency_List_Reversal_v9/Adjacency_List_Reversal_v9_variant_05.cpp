#include <iostream>
#include <cstdlib>
#include <ctime>

#define MAXV 8          // maximum vertices
#define MAXE 20         // maximum total edges
#define MAXD 5          // maximum out-degree per vertex

/* LLM input variant 5: duplicate-heavy */

// ------------------------------------------------------------
// Generate a random directed graph using stack‑allocated arrays
// ------------------------------------------------------------
void genGraph(int vcnt, int deg[], int adj[][MAXD]) {
    // initialise degrees
    for (int i = 0; i < vcnt; ++i) deg[i] = 0;

    // fill with random edges, avoiding self‑loops and duplicates
    for (int e = 0; e < MAXE; ++e) {
        int src = std::rand() % vcnt;
        int dst = std::rand() % vcnt;
        if (src == dst) continue;                         // no self‑loop
        // check duplicate
        bool dup = false;
        for (int k = 0; k < deg[src]; ++k)
            if (adj[src][k] == dst) { dup = true; break; }
        if (dup) continue;
        // store if degree permits
        if (deg[src] < MAXD) adj[src][deg[src]++] = dst;
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

    // Manual duplicate‑heavy initialization
    for (int i = 0; i < vertices; ++i) {
        outDeg[i] = 4; // use 4 entries per vertex (within MAXD)
        outAdj[i][0] = (i + 1) % vertices;
        outAdj[i][1] = (i + 1) % vertices; // duplicate
        outAdj[i][2] = (i + 2) % vertices;
        outAdj[i][3] = (i + 2) % vertices; // duplicate
    }
    // Zero‑initialize any remaining slots
    for (int i = vertices; i < MAXV; ++i) outDeg[i] = 0;

    printGraph("Original graph", vertices, outDeg, outAdj);

    int revDeg[MAXV];                      // reverse out‑degree
    int revAdj[MAXV][MAXD];                // reversed adjacency list

    revGraph(vertices, outDeg, outAdj, revDeg, revAdj);
    printGraph("Reversed graph", vertices, revDeg, revAdj);

    return 0;
}
