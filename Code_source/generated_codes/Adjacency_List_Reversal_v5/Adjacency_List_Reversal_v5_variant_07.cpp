#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 7: reverse-adversarial */

// generate a deterministic worst‑case directed graph (adjacency list)
// each vertex has maximum out‑degree and edges point in reverse order
void generateGraph(int n, int** &adj, int* &deg) {
    adj = new int*[n];
    deg = new int[n];
    int v = 0;
    while (v < n) {
        int d = n;               // maximum possible out‑degree
        deg[v] = d;
        adj[v] = new int[d];
        int e = 0;
        while (e < d) {
            // targets in decreasing order to create a reverse‑ordered pattern
            adj[v][e] = n - 1 - e;
            ++e;
        }
        ++v;
    }
}

// first pass: compute indegrees of the reversed graph
void computeReverseDegrees(int n, int** adj, int* deg, int* &revDeg) {
    revDeg = new int[n];
    int i = 0;
    while (i < n) {
        revDeg[i] = 0;
        ++i;
    }
    i = 0;
    while (i < n) {
        int d = deg[i];
        int j = 0;
        // manual unrolling by 2
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
        ++i;
    }
}

// second pass: fill the reversed adjacency lists
void buildReverseGraph(int n, int** adj, int* deg, int* revDeg, int** &revAdj) {
    revAdj = new int*[n];
    int i = 0;
    while (i < n) {
        int d = revDeg[i];
        if (d > 0) {
            revAdj[i] = new int[d];
        } else {
            revAdj[i] = nullptr;
        }
        ++i;
    }

    // temporary counters for each vertex
    int* pos = new int[n];
    i = 0;
    while (i < n) {
        pos[i] = 0;
        ++i;
    }

    i = 0;
    while (i < n) {
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
        ++i;
    }
    delete[] pos;
}

// print adjacency list
void printAdjacency(int n, int** adj, int* deg, char* label) {
    std::cout << label << " adjacency list:\n";
    int v = 0;
    while (v < n) {
        std::cout << v << ": ";
        int d = deg[v];
        int e = 0;
        while (e < d) {
            std::cout << adj[v][e] << " ";
            ++e;
        }
        std::cout << "\n";
        ++v;
    }
}

// free allocated memory
void freeGraph(int n, int** adj, int* deg) {
    int v = 0;
    while (v < n) {
        if (adj[v] != nullptr) {
            delete[] adj[v];
        }
        ++v;
    }
    delete[] adj;
    delete[] deg;
}

// Version 5
int main() {
    std::srand((int)std::time(0));   // seed RNG (not used in this variant)
    int nodeCount = 8;               // larger example for adversarial pattern

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
