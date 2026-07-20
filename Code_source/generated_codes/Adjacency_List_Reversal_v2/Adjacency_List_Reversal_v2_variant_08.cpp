#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 8: sparse-skewed */
int main() {
    // ----- parameters -----
    int V = 8;                     // number of vertices
    int MXD = 5;                   // max degree per vertex
    int outDeg[10] = {0};          // out-degree counters (stack array)
    int outAdj[10][5];             // adjacency lists (stack array)
    int revDeg[10] = {0};          // reverse graph degree counters
    int revAdj[10][5];             // reverse adjacency lists

    // ----- deterministic sparse, skewed graph -----
    // Vertex 0 has maximum out-degree, others are mostly empty
    outDeg[0] = 5;
    outAdj[0][0] = 1;
    outAdj[0][1] = 2;
    outAdj[0][2] = 3;
    outAdj[0][3] = 4;
    outAdj[0][4] = 5;

    outDeg[1] = 0;
    outDeg[2] = 0;
    outDeg[3] = 0;

    outDeg[4] = 1;
    outAdj[4][0] = 0;

    outDeg[5] = 0;
    outDeg[6] = 0;
    outDeg[7] = 0;

    // ----- build reversed graph -----
    for (int i = 0; i < V; ++i) {
        for (int j = 0; j < outDeg[i]; ++j) {
            int nb = outAdj[i][j];
            // add edge nb -> i in reversed graph
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
