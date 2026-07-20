#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // ----- parameters -----
    int V = 6;                     // number of vertices
    int MXD = 5;                   // max degree per vertex
    int outDeg[10] = {0};          // out-degree counters (stack array)
    int outAdj[10][5];             // adjacency lists (stack array)
    int revDeg[10] = {0};          // reverse graph degree counters
    int revAdj[10][5];             // reverse adjacency lists

    // ----- generate deterministic directed graph -----
    int i = 0;
    while (i < V) {
        // deterministic degree between 0 and MXD
        outDeg[i] = (i * 7 + 3) % (MXD + 1);
        int j = 0;
        while (j < outDeg[i]) {
            // deterministic neighbor (allow self‑loops)
            outAdj[i][j] = (i * 5 + j * 7 + 2) % V;
            ++j;
        }
        ++i;
    }

    // ----- build reversed graph -----
    i = 0;
    while (i < V) {
        int j = 0;
        while (j < outDeg[i]) {
            int nb = outAdj[i][j];
            // add edge nb -> i in reversed graph
            revAdj[nb][ revDeg[nb]++ ] = i;
            ++j;
        }
        ++i;
    }

    // ----- print original adjacency list -----
    cout << "Original graph adjacency list:\n";
    i = 0;
    while (i < V) {
        cout << i << ": ";
        int j = 0;
        while (j < outDeg[i]) {
            cout << outAdj[i][j] << (j + 1 < outDeg[i] ? " " : "");
            ++j;
        }
        cout << "\n";
        ++i;
    }

    // ----- print reversed adjacency list -----
    cout << "\nReversed graph adjacency list:\n";
    i = 0;
    while (i < V) {
        cout << i << ": ";
        int j = 0;
        while (j < revDeg[i]) {
            cout << revAdj[i][j] << (j + 1 < revDeg[i] ? " " : "");
            ++j;
        }
        cout << "\n";
        ++i;
    }

    return 0;
}
