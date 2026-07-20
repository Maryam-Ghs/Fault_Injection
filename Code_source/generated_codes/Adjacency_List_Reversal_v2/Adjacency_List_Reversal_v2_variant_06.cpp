#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 6: ordered-structured */

int main() {
    // ----- parameters -----
    int V = 6;                     // number of vertices
    int MXD = 5;                   // max degree per vertex
    int outDeg[10] = {0};          // out-degree counters (stack array)
    int outAdj[10][5];             // adjacency lists (stack array)
    int revDeg[10] = {0};          // reverse graph degree counters
    int revAdj[10][5];             // reverse adjacency lists

    // ----- generate ordered structured directed graph -----
    for (int i = 0; i < V; ++i) {
        // each vertex connects to the next MXD vertices (or fewer if at end)
        outDeg[i] = min(MXD, V - i - 1);
        for (int j = 0; j < outDeg[i]; ++j) {
            outAdj[i][j] = i + j + 1; // forward neighbor
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
