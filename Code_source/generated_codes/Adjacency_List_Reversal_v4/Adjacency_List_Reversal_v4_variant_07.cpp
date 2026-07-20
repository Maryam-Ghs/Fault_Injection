/* LLM input variant 7: reverse-adversarial */
// Adjacency List Reversal – version 4
// Implements graph reversal using heap‑allocated arrays, manual loop unrolling and fused expressions.
// No use of double, long, unsigned or const. Input is generated internally.

#include <iostream>

using namespace std;

// ---------- Helper: compute out‑degrees ----------
void calcOutDeg(int vCount, int eCount, int* srcArr, int* outDeg) {
    int i = 0;
    while (i < vCount) outDeg[i++] = 0;                 // initialise
    i = 0;
    while (i < eCount) outDeg[srcArr[i]]++, i++;        // fused ++
}

// ---------- Helper: allocate adjacency lists ----------
int** allocAdj(int vCount, int* degArr) {
    int** adj = new int*[vCount];
    int i = 0;
    while (i < vCount) {
        adj[i] = (degArr[i] > 0) ? new int[degArr[i]] : nullptr;
        i++;
    }
    return adj;
}

// ---------- Helper: fill adjacency lists ----------
void fillAdj(int eCount, int* srcArr, int* dstArr,
             int** adj, int* curPos) {
    int i = 0;
    while (i < eCount) {
        int s = srcArr[i];
        int d = dstArr[i];
        adj[s][curPos[s]++] = d;        // fused assignment & increment
        i++;
    }
}

// ---------- Helper: reverse the graph ----------
void reverseGraph(int vCount, int eCount,
                  int* srcArr, int* dstArr,
                  int** revAdj, int* revPos) {
    int i = 0;
    while (i < eCount) {
        int s = srcArr[i];
        int d = dstArr[i];
        revAdj[d][revPos[d]++] = s;     // insert reversed edge
        i++;
    }
}

// ---------- Helper: print adjacency list with manual unrolling ----------
void printGraph(const char* title, int vCount,
                int** adj, int* degArr) {
    cout << title << endl;
    int v = 0;
    while (v < vCount) {
        cout << "  " << v << ": ";
        int k = 0;
        // unroll two at a time
        while (k + 1 < degArr[v]) {
            cout << adj[v][k] << " " << adj[v][k+1] << " ";
            k += 2;
        }
        // handle possible leftover
        if (k < degArr[v]) {
            cout << adj[v][k] << " ";
        }
        cout << endl;
        v++;
    }
    cout << endl;
}

// ---------- Main ----------
int main() {
    // deterministic adversarial test graph (complete directed graph without self‑loops)
    int vCount = 8;
    int eCount = 56;
    int srcArr[56] = {
        7,7,7,7,7,7,7,
        6,6,6,6,6,6,6,
        5,5,5,5,5,5,5,
        4,4,4,4,4,4,4,
        3,3,3,3,3,3,3,
        2,2,2,2,2,2,2,
        1,1,1,1,1,1,1,
        0,0,0,0,0,0,0
    };
    int dstArr[56] = {
        6,5,4,3,2,1,0,
        7,5,4,3,2,1,0,
        7,6,4,3,2,1,0,
        7,6,5,3,2,1,0,
        7,6,5,4,2,1,0,
        7,6,5,4,3,1,0,
        7,6,5,4,3,2,0,
        7,6,5,4,3,2,1
    };

    // ----- original graph -----
    int* outDeg = new int[vCount];
    calcOutDeg(vCount, eCount, srcArr, outDeg);

    int** adj = allocAdj(vCount, outDeg);

    // position markers for filling
    int* curPos = new int[vCount];
    int i = 0;
    while (i < vCount) curPos[i++] = 0;
    fillAdj(eCount, srcArr, dstArr, adj, curPos);

    // ----- reversed graph -----
    // in-degree = out-degree of reversed graph
    int* revDeg = new int[vCount];
    calcOutDeg(vCount, eCount, dstArr, revDeg);   // reuse degree calc with swapped arrays

    int** revAdj = allocAdj(vCount, revDeg);

    int* revPos = new int[vCount];
    i = 0;
    while (i < vCount) revPos[i++] = 0;
    reverseGraph(vCount, eCount, srcArr, dstArr, revAdj, revPos);

    // ----- output -----
    printGraph("Original adjacency lists:", vCount, adj, outDeg);
    printGraph("Reversed adjacency lists:", vCount, revAdj, revDeg);

    // ----- cleanup -----
    i = 0;
    while (i < vCount) {
        delete[] adj[i];
        delete[] revAdj[i];
        i++;
    }
    delete[] adj;
    delete[] revAdj;
    delete[] outDeg;
    delete[] revDeg;
    delete[] curPos;
    delete[] revPos;

    return 0;
}
