#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 3: zeros-and-ones */

int main() {
    // ----- parameters -----
    int V = 6;                     // number of vertices
    int MXD = 5;                   // max degree per vertex
    int outDeg[10] = {0};          // out-degree counters (stack array)
    int outAdj[10][5];             // adjacency lists (stack array)
    int revDeg[10] = {0};          // reverse graph degree counters
    int revAdj[10][5];             // reverse adjacency lists

    // ----- deterministic graph emphasizing zeros and ones -----
    // Define degrees: 0 or 1
    outDeg[0] = 1;
    outDeg[1] = 0;
    outDeg[2] = 1;
    outDeg[3] = 0;
    outDeg[4] = 1;
    outDeg[5] = 0;

    // Define adjacency entries using 0 and 1 (including self‑loops)
    outAdj[0][0] = 0;   // self-loop
    outAdj[2][0] = 1;
    outAdj[4][0] = 0;

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
