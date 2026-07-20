#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 2: small-diverse */

int main() {
    // ----- parameters -----
    int V = 6;                     // number of vertices
    int MXD = 5;                   // max degree per vertex
    int outDeg[10] = {0};          // out-degree counters (stack array)
    int outAdj[10][5];             // adjacency lists (stack array)
    int revDeg[10] = {0};          // reverse graph degree counters
    int revAdj[10][5];             // reverse adjacency lists

    // ----- deterministic directed graph -----
    // Vertex 0: edges to 1, 2, 0 (self‑loop)
    outDeg[0] = 3;
    outAdj[0][0] = 1;
    outAdj[0][1] = 2;
    outAdj[0][2] = 0;

    // Vertex 1: edges to 3, 5
    outDeg[1] = 2;
    outAdj[1][0] = 3;
    outAdj[1][1] = 5;

    // Vertex 2: edges to 0, 2 (self‑loop), 4, 5
    outDeg[2] = 4;
    outAdj[2][0] = 0;
    outAdj[2][1] = 2;
    outAdj[2][2] = 4;
    outAdj[2][3] = 5;

    // Vertex 3: edge to 1
    outDeg[3] = 1;
    outAdj[3][0] = 1;

    // Vertex 4: no outgoing edges
    outDeg[4] = 0;

    // Vertex 5: edges to 0, 1, 2, 3, 5 (self‑loop)
    outDeg[5] = 5;
    outAdj[5][0] = 0;
    outAdj[5][1] = 1;
    outAdj[5][2] = 2;
    outAdj[5][3] = 3;
    outAdj[5][4] = 5;

    // ----- build reversed graph -----
    for (int i = 0; i < V; ++i) {
        for (int j = 0; j < outDeg[i]; ++j) {
            int nb = outAdj[i][j];
            revAdj[nb][ revDeg[nb]++ ] = i;
        }
    }

    // ----- print original adjacency list -----
    cout << "Original graph adjacency list:\n";
    for (int i = 0; i < V; ++i) {
        cout << i << ": ";
        for (int j = 0; j < outDeg[i]; ++j) {
            cout << outAdj[i][j] << (j + 1 < outDeg[i] ? " " : "");
        }
        cout << "\n";
    }

    // ----- print reversed adjacency list -----
    cout << "\nReversed graph adjacency list:\n";
    for (int i = 0; i < V; ++i) {
        cout << i << ": ";
        for (int j = 0; j < revDeg[i]; ++j) {
            cout << revAdj[i][j] << (j + 1 < revDeg[i] ? " " : "");
        }
        cout << "\n";
    }

    return 0;
}
