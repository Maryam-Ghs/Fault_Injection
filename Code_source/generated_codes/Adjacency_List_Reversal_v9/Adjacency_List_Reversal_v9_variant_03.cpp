#include <iostream>
#include <cstdlib>
#include <ctime>

#define MAXV 8          // maximum vertices
#define MAXE 20         // maximum total edges
#define MAXD 5          // maximum out-degree per vertex

/* LLM input variant 3: zeros-and-ones */

// ------------------------------------------------------------
// Generate a deterministic graph emphasizing zeros and ones
// ------------------------------------------------------------
void genGraph(int vcnt, int deg[], int adj[][MAXD]) {
    // initialise degrees
    for (int i = 0; i < vcnt; ++i) deg[i] = 0;

    // create edges only between vertex 0 and vertex 1
    if (vcnt > 1) {
        // 0 -> 1
        adj[0][deg[0]++] = 1;
        // 1 -> 0
        adj[1][deg[1]++] = 0;
    }
    // remaining vertices keep degree 0 (empty-like)
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
    std::srand(12345);                     // deterministic pseudo‑random (unused)

    int vertices = 4;                      // small graph with empty vertices
    int outDeg[MAXV];                      // out‑degree per vertex
    int outAdj[MAXV][MAXD];                // adjacency list (stack)

    genGraph(vertices, outDeg, outAdj);    // create deterministic graph
    printGraph("Original graph", vertices, outDeg, outAdj);

    int revDeg[MAXV];                      // reverse out‑degree
    int revAdj[MAXV][MAXD];                // reversed adjacency list

    revGraph(vertices, outDeg, outAdj, revDeg, revAdj);
    printGraph("Reversed graph", vertices, revDeg, revAdj);

    return 0;
}
