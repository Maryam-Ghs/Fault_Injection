/* LLM input variant 1: minimal-boundary */
#include <bits/stdc++.h>
using namespace std;

int main() {
    // ----- parameters -----
    int V = 2;                     // minimal number of vertices for a non‑trivial graph
    int MXD = 1;                   // minimal max degree (boundary value)
    int outDeg[10] = {0};          // out-degree counters (stack array)
    int outAdj[10][5];             // adjacency lists (stack array)
    int revDeg[10] = {0};          // reverse graph degree counters
    int revAdj[10][5];             // reverse adjacency lists

    // ----- deterministic small directed graph -----
    // Vertex 0 -> Vertex 1, Vertex 1 has no outgoing edges
    outDeg[0] = 1;
    outAdj[0][0] = 1;
    outDeg[1] = 0; // no edges from vertex 1

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
