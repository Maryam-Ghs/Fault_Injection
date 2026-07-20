#include <iostream>

/* LLM input variant 2: small-diverse */

#define MAXV 5          // maximum vertices
#define MAXE 10         // maximum total edges (unused in this variant)
#define MAXD 4          // maximum out-degree per vertex

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
// Entry point – version #9 (variant 2)
// ------------------------------------------------------------
int main() {
    int vertices = 5;                      // small graph with diverse structure
    int outDeg[MAXV] = {0};                 // out‑degree per vertex
    int outAdj[MAXV][MAXD] = {{0}};          // adjacency list (stack)

    // Manually define a diverse set of edges
    // Vertex 0 -> 1, 3
    outDeg[0] = 2;
    outAdj[0][0] = 1;
    outAdj[0][1] = 3;

    // Vertex 1 -> 4
    outDeg[1] = 1;
    outAdj[1][0] = 4;

    // Vertex 2 -> (no outgoing edges)
    outDeg[2] = 0;

    // Vertex 3 -> 0, 2, 4
    outDeg[3] = 3;
    outAdj[3][0] = 0;
    outAdj[3][1] = 2;
    outAdj[3][2] = 4;

    // Vertex 4 -> 0
    outDeg[4] = 1;
    outAdj[4][0] = 0;

    printGraph("Original graph", vertices, outDeg, outAdj);

    int revDeg[MAXV] = {0};                 // reverse out‑degree
    int revAdj[MAXV][MAXD] = {{0}};         // reversed adjacency list

    revGraph(vertices, outDeg, outAdj, revDeg, revAdj);
    printGraph("Reversed graph", vertices, revDeg, revAdj);

    return 0;
}
