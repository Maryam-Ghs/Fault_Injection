/ * LLM input variant 10: large-safe-stress * /
#include <bits/stdc++.h>
using namespace std;

int main() {
    // ----- parameters -----
    const int V = 10;                // number of vertices (max safe size)
    const int MXD = 5;                // max degree per vertex (fits array)
    int outDeg[10] = {0};             // out-degree counters
    int outAdj[10][5];                // adjacency lists
    int revDeg[10] = {0};             // reverse graph degree counters
    int revAdj[10][5];                // reverse adjacency lists

    // ----- deterministic directed graph generation -----
    for (int i = 0; i < V; ++i) {
        outDeg[i] = MXD;              // each vertex has maximum allowed degree
        for (int j = 0; j < MXD; ++j) {
            // connect to the next MXD vertices cyclically (no self‑loops)
            outAdj[i][j] = (i + j + 1) % V;
        }
    }

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
