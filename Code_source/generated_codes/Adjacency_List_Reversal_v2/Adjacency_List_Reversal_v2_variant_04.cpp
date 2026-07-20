#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 4: signed-extremes */

int main() {
    // ----- parameters -----
    int V = 6;                     // number of vertices
    int MXD = 5;                   // max degree per vertex
    int outDeg[10] = {0};          // out-degree counters (stack array)
    int outAdj[10][5];             // adjacency lists (stack array)
    int revDeg[10] = {0};          // reverse graph degree counters
    int revAdj[10][5];             // reverse adjacency lists

    // ----- deterministic graph with mixed signed degrees -----
    // Negative, zero, and positive out-degree values
    outDeg[0] = -2;  // treated as no outgoing edges
    outDeg[1] = 0;   // no outgoing edges
    outDeg[2] = 3;   // three outgoing edges
    outDeg[3] = -1;  // treated as no outgoing edges
    outDeg[4] = 5;   // maximum allowed outgoing edges
    outDeg[5] = 1;   // single outgoing edge

    // Fill adjacency lists only for vertices with non‑negative degree
    // Vertex 2 edges
    outAdj[2][0] = 0;
    outAdj[2][1] = 2; // self‑loop
    outAdj[2][2] = 5;
    // Vertex 4 edges
    outAdj[4][0] = 1;
    outAdj[4][1] = 3;
    outAdj[4][2] = 4; // self‑loop
    outAdj[4][3] = 0;
    outAdj[4][4] = 5;
    // Vertex 5 edge
    outAdj[5][0] = 2;

    // ----- build reversed graph -----
    int i = 0;
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
