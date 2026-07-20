#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */

#define MAXV 2          // maximum vertices (minimal)
#define MAXE 1          // maximum total edges (minimal)
#define MAXD 1          // maximum out-degree per vertex (boundary)

void genGraph(int vcnt, int deg[], int adj[][MAXD]) {
    for (int i = 0; i < vcnt; ++i) deg[i] = 0;
    for (int e = 0; e < MAXE; ++e) {
        int src = std::rand() % vcnt;
        int dst = std::rand() % vcnt;
        if (src == dst) continue;
        bool dup = false;
        for (int k = 0; k < deg[src]; ++k)
            if (adj[src][k] == dst) { dup = true; break; }
        if (dup) continue;
        if (deg[src] < MAXD) adj[src][deg[src]++] = dst;
    }
}

void revGraph(int vcnt, int deg[], int adj[][MAXD],
              int rdeg[], int radj[][MAXD]) {
    for (int i = 0; i < vcnt; ++i) rdeg[i] = 0;
    for (int u = 0; u < vcnt; ++u) {
        for (int i = 0; i < deg[u]; ++i) {
            int v = adj[u][i];
            radj[v][rdeg[v]++] = u;
        }
    }
}

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

int main() {
    std::srand(12345);                     // deterministic pseudo‑random

    int vertices = 2;                      // minimal nontrivial graph
    int outDeg[MAXV];                      
    int outAdj[MAXV][MAXD];                

    // deterministic minimal graph: edge 0 -> 1
    for (int i = 0; i < vertices; ++i) outDeg[i] = 0;
    outDeg[0] = 1;
    outAdj[0][0] = 1;
    outDeg[1] = 0;

    printGraph("Original graph", vertices, outDeg, outAdj);

    int revDeg[MAXV];                      
    int revAdj[MAXV][MAXD];                

    revGraph(vertices, outDeg, outAdj, revDeg, revAdj);
    printGraph("Reversed graph", vertices, revDeg, revAdj);

    return 0;
}
