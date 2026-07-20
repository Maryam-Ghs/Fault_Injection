#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 8: sparse-skewed */

// generate a deterministic sparse and skewed directed graph (adjacency list)
void generateGraph(int n, int** &adj, int* &deg) {
    adj = new int*[n];
    deg = new int[n];
    for (int v = 0; v < n; ++v) {
        int d = 0;
        if (v == 0) {
            // hub vertex with edges to all other vertices
            d = n - 1;
        } else if (v == 1) {
            // moderately connected vertex
            d = n / 2;
        } else if (v == 2) {
            // single outgoing edge
            d = 1;
        } else {
            // remaining vertices have no outgoing edges
            d = 0;
        }
        deg[v] = d;
        if (d > 0) {
            adj[v] = new int[d];
            for (int e = 0; e < d; ++e) {
                // deterministic target selection to keep sparsity
                if (v == 0) {
                    // connect to every other vertex except self
                    adj[v][e] = (e >= v) ? e + 1 : e;
                } else if (v == 1) {
                    // connect to first d vertices (including self for demonstration)
                    adj[v][e] = e;
                } else if (v == 2) {
                    adj[v][e] = 0; // edge back to hub
                }
            }
        } else {
            adj[v] = nullptr;
        }
    }
}

// first pass: compute indegrees of the reversed graph
void computeReverseDegrees(int n, int** adj, int* deg, int* &revDeg) {
    revDeg = new int[n];
    for (int i = 0; i < n; ++i) {
        revDeg[i] = 0;
    }
    for (int i = 0; i < n; ++i) {
        int d = deg[i];
        int j = 0;
        while (j + 1 < d) {
            int t1 = adj[i][j];
            int t2 = adj[i][j + 1];
            revDeg[t1] = revDeg[t1] + 1;
            revDeg[t2] = revDeg[t2] + 1;
            j = j + 2;
        }
        if (j < d) {
            int t = adj[i][j];
            revDeg[t] = revDeg[t] + 1;
        }
    }
}

// second pass: fill the reversed adjacency lists
void buildReverseGraph(int n, int** adj, int* deg, int* revDeg, int** &revAdj) {
    revAdj = new int*[n];
    for (int i = 0; i < n; ++i) {
        int d = revDeg[i];
        if (d > 0) {
            revAdj[i] = new int[d];
        } else {
            revAdj[i] = nullptr;
        }
    }

    // temporary counters for each vertex
    int* pos = new int[n];
    for (int i = 0; i < n; ++i) {
        pos[i] = 0;
    }

    for (int i = 0; i < n; ++i) {
        int d = deg[i];
        int j = 0;
        while (j + 1 < d) {
            int t1 = adj[i][j];
            int t2 = adj[i][j + 1];
            revAdj[t1][pos[t1]++] = i;
            revAdj[t2][pos[t2]++] = i;
            j = j + 2;
        }
        if (j < d) {
            int t = adj[i][j];
            revAdj[t][pos[t]++] = i;
        }
    }
    delete[] pos;
}

// print adjacency list
void printAdjacency(int n, int** adj, int* deg, char* label) {
    std::cout << label << " adjacency list:\n";
    for (int v = 0; v < n; ++v) {
        std::cout << v << ": ";
        int d = deg[v];
        for (int e = 0; e < d; ++e) {
            std::cout << adj[v][e] << " ";
        }
        std::cout << "\n";
    }
}

// free allocated memory
void freeGraph(int n, int** adj, int* deg) {
    for (int v = 0; v < n; ++v) {
        if (adj[v] != nullptr) {
            delete[] adj[v];
        }
    }
    delete[] adj;
    delete[] deg;
}

// Version 5
int main() {
    std::srand((int)std::time(0));   // seed RNG (not used in this variant)
    int nodeCount = 10;               // larger example to showcase sparsity and skew

    int** outAdj;
    int* outDeg;
    generateGraph(nodeCount, outAdj, outDeg);

    int* revDeg;
    computeReverseDegrees(nodeCount, outAdj, outDeg, revDeg);

    int** revAdj;
    buildReverseGraph(nodeCount, outAdj, outDeg, revDeg, revAdj);

    char title1[] = "Original";
    char title2[] = "Reversed";
    printAdjacency(nodeCount, outAdj, outDeg, title1);
    printAdjacency(nodeCount, revAdj, revDeg, title2);

    freeGraph(nodeCount, outAdj, outDeg);
    freeGraph(nodeCount, revAdj, revDeg);
    return 0;
}
