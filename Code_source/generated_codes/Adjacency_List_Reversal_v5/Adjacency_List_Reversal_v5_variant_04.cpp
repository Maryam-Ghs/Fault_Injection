#include <iostream>
/* LLM input variant 4: signed-extremes */

// deterministic graph generation with mixed negative, zero, and positive degrees
void generateGraph(int n, int** &adj, int* &deg) {
    adj = new int*[n];
    deg = new int[n];
    // predefined degrees (mix of negative, zero, positive)
    int predefinedDeg[6] = { -2, 0, 3, -1, 2, 0 };
    // predefined adjacency lists for vertices with positive degree
    int predefinedAdj0[3] = { 0, 1, 5 }; // for vertex 2
    int predefinedAdj1[2] = { 2, 4 };    // for vertex 4

    for (int v = 0; v < n; ++v) {
        int d = predefinedDeg[v];
        deg[v] = d;
        if (d > 0) {
            adj[v] = new int[d];
            if (v == 2) {
                for (int e = 0; e < d; ++e) adj[v][e] = predefinedAdj0[e];
            } else if (v == 4) {
                for (int e = 0; e < d; ++e) adj[v][e] = predefinedAdj1[e];
            } else {
                // fallback (should not occur with current data)
                for (int e = 0; e < d; ++e) adj[v][e] = 0;
            }
        } else {
            adj[v] = nullptr;
        }
    }
}

// first pass: compute indegrees of the reversed graph
void computeReverseDegrees(int n, int** adj, int* deg, int* &revDeg) {
    revDeg = new int[n];
    for (int i = 0; i < n; ++i) revDeg[i] = 0;
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
        revAdj[i] = (d > 0) ? new int[d] : nullptr;
    }

    int* pos = new int[n];
    for (int i = 0; i < n; ++i) pos[i] = 0;

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
void printAdjacency(int n, int** adj, int* deg, const char* label) {
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
        if (adj[v] != nullptr) delete[] adj[v];
    }
    delete[] adj;
    delete[] deg;
}

// Version 5
int main() {
    int nodeCount = 6; // deterministic small graph

    int** outAdj;
    int* outDeg;
    generateGraph(nodeCount, outAdj, outDeg);

    int* revDeg;
    computeReverseDegrees(nodeCount, outAdj, outDeg, revDeg);

    int** revAdj;
    buildReverseGraph(nodeCount, outAdj, outDeg, revDeg, revAdj);

    const char title1[] = "Original";
    const char title2[] = "Reversed";
    printAdjacency(nodeCount, outAdj, outDeg, title1);
    printAdjacency(nodeCount, revAdj, revDeg, title2);

    freeGraph(nodeCount, outAdj, outDeg);
    freeGraph(nodeCount, revAdj, revDeg);
    return 0;
}
